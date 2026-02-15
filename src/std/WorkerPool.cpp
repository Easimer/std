/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/WorkerPool.hpp"
#include "std/Arena.h"
#include "std/Check.h"
#include "std/SignalTree.hpp"
#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"
#include "std/Types.h"
#include "std/os/Thread.hpp"

#include <atomic>

struct WorkContract {
  alignas(64) impl::AtomicU32 numPending;
  KernelEntryPoint entryPoint;
  const char *label;
  u32 flags = 0;

  void notifyJobDispatched() { numPending.fetchAdd(1); }
  bool notifyJobFinished() {
    u32 prev = numPending.fetchSub(1);

    if (prev == 1) {
      numPending.notifyAll();
      return true;
    }

    return false;
  }

  void wait() {
    u32 c = numPending.load();
    while (c != 0) {
      numPending.wait(c);
      c = numPending.load();
    }
  }
};

struct Job {
  std::atomic<bool> occupied;
  bool isPriority;

  WorkContract *workContract;
  void *parameters;
  ThreadIndex x;
};

struct SignalTreeAndJobs {
  std::atomic<bool> shutdown;

  SignalTree<10> signalTree;
  Job jobs[SignalTree<10>::NumLeafNodes];
};

struct WorkerThreadProcInfo {
  SignalTreeAndJobs *stj;
  u32 idxThread;
  WorkerPool *workerPool;
};

static void workerThreadProc(void *arg) {
  auto [stj, idxThread, workerPool] =
      *reinterpret_cast<WorkerThreadProcInfo *>(arg);

  while (!stj->shutdown) {
    // Try to decrement the root node. If we can do that, we'll have a guarantee
    // that a path exists in the tree to a job
    bool ok = false;
    for (u32 numAttempts = 0; numAttempts < 64; numAttempts++) {
      u32 rootCount = stj->signalTree.nodes[0].counter.load();
      if (rootCount == 0) {
        break;
      }

      if (!stj->signalTree.nodes[0].counter.compareExchange(rootCount,
                                                            rootCount - 1)) {
        continue;
      }
      ok = true;
      break;
    }

    if (!ok) {
      // Couldn't decrement the root node; give up and go to sleep
      stj->signalTree.waitForRootToChange();
      continue;
    }

    if (stj->shutdown) {
      break;
    }

    u32 idxNode = 0;
    for (u32 idxLevel = 1; idxLevel < stj->signalTree.numLevels(); idxLevel++) {
      // Try the left side
      u32 idxLeftChild = impl::eytzingerLeft(idxNode);
      u32 idxRightChild = impl::eytzingerRight(idxNode);

      if (stj->signalTree.tryDecrementPriority(idxNode, idxLeftChild,
                                               idxRightChild, idxNode)) {
        continue;
      }

      if (stj->signalTree.tryDecrementNode(idxLeftChild)) {
        idxNode = idxLeftChild;
        continue;
      }

      if (stj->signalTree.tryDecrementNode(idxRightChild)) {
        idxNode = idxRightChild;
        continue;
      }

      NOTREACHED();
    }

    DCHECK(stj->signalTree.isLeafNode(idxNode));
    u32 idxJob = idxNode - SignalTree<10>::IdxFirstLeafNode;
    Job &job = stj->jobs[idxJob];
    DCHECK(job.occupied == true);

    void *parameters = job.parameters;
    Dispatch dispatch = {parameters, job.x, idxThread, workerPool};

    WorkContract *wc = job.workContract;
    wc->entryPoint(&dispatch);

    bool wcOwned = wc->flags & INTERNAL_WC_OWNED;
    bool contractIsFinished = wc->notifyJobFinished();

    if (contractIsFinished && wcOwned) {
      delete[] reinterpret_cast<u8 *>(parameters);
    }

    if (job.isPriority) {
      stj->signalTree.clearPriorityForJob(idxJob);
    }
    job.occupied.store(false, std::memory_order_release);
  }
}

struct WorkerPoolImpl final : WorkerPool {
  const Slice<Thread> threads;
  SignalTreeAndJobs signalTreeAndJobs;
  u32 idxLastJobSlotUsed;

  WorkerPoolImpl(Slice<Thread> threads)
      : threads(threads), signalTreeAndJobs(), idxLastJobSlotUsed(0) {}

  WorkContract *createWorkContract(Arena *arena,
                                   KernelEntryPoint entry,
                                   u32 flags = 0) override {
    static constexpr u32 INTERNAL_MASK = INTERNAL_WC_OWNED;
    DCHECK((flags & INTERNAL_MASK) == 0);
    flags &= ~INTERNAL_MASK;

    WorkContract *wc = alloc<WorkContract>(arena);
    wc->numPending.store(0);
    wc->entryPoint = entry;
    wc->label = nullptr;
    wc->flags = flags;
    return wc;
  }

  void dispatch(WorkContract *workContract,
                void *parameters,
                u32 numThreadsX,
                u32 numThreadsY,
                u32 numThreadsZ) override {
    DCHECK(workContract);

    const bool isPriority = (workContract->flags & WC_HIGH_PRIORITY) != 0;

    for (u32 z = 0; z < numThreadsZ; z++) {
      for (u32 y = 0; y < numThreadsY; y++) {
        for (u32 x = 0; x < numThreadsX; x++) {
          ThreadIndex i = {x, y, z};
          dispatchJob(workContract, parameters, i, isPriority);
        }
      }
    }

    signalTreeAndJobs.signalTree.notifyAll();
  }

  void release(WorkContract *workContract) override {
    workContract->wait();
    if (workContract->flags & INTERNAL_WC_OWNED) {
      // If the contract is finished and the contract memory is owned by us,
      // free it
      delete workContract;
    }
  }

  /** Finds a free job slot, marks it as occupied and returns its index. */
  u32 allocateJobIndex() {
    constexpr u32 Mask = SignalTree<10>::NodeIndexMask;

    // Start from one past the previously used slot
    u32 idxJob = (idxLastJobSlotUsed + 1) & Mask;
    const u32 idxFirstJob = idxJob;
    while (true) {
      if (signalTreeAndJobs.jobs[idxJob].occupied) {
        idxJob = (idxJob + 1) & Mask;
        // Slots are probably all full; wake up every thread
        if (idxJob == idxFirstJob) {
          signalTreeAndJobs.signalTree.notifyAll();
        }
        continue;
      }

      // Try mark as occupied
      bool expected = false;
      Job &job = signalTreeAndJobs.jobs[idxJob];
      if (!job.occupied.compare_exchange_strong(expected, true,
                                                std::memory_order_acquire)) {
        continue;
      }

      return idxJob;
    }
  }

  void dispatchJob(WorkContract *workContract,
                   void *parameters,
                   ThreadIndex x,
                   bool isPriority) {
    u32 idxJob = allocateJobIndex();

    Job &job = signalTreeAndJobs.jobs[idxJob];
    job.workContract = workContract;
    job.isPriority = isPriority;
    job.x = x;
    job.parameters = parameters;
    workContract->notifyJobDispatched();

    // Set the signal for this job
    signalTreeAndJobs.signalTree.setSignalForJob(idxJob, isPriority);
    idxLastJobSlotUsed = idxJob;
  }

  void shutdown() override {
    // Set the shutdown flag
    signalTreeAndJobs.shutdown.store(true, std::memory_order_seq_cst);
    // Wake up all threads
    signalTreeAndJobs.signalTree.setRoot(threads.length);

    // Wait for threads to react to the shutdown flag
    for (auto [t, _] : threads) {
      t.join();
    }
  }
};

WorkerPool *createWorkerPool(Arena *arena, u32 numThreads) {
  Slice<Thread> threads;
  alloc(arena, numThreads, threads);
  Slice<WorkerThreadProcInfo> threadProcInfo;
  alloc(arena, numThreads, threadProcInfo);

  WorkerPoolImpl *wp = alloc<WorkerPoolImpl>(arena);

  new (wp) WorkerPoolImpl(threads);

  for (auto [_, i] : threadProcInfo) {
    threadProcInfo[i].stj = &wp->signalTreeAndJobs;
    threadProcInfo[i].idxThread = i;
    threadProcInfo[i].workerPool = wp;
  }

  for (auto [_, i] : threads) {
    Result<Thread, ThreadError> res = Thread::create({
        .nextInChain = nullptr,
        .entryPoint = workerThreadProc,
        .param = &threadProcInfo[i],
    });
    CHECK(res.isOk());
    threads[i] = res.unwrap();
  }

  return wp;
}

WorkerPool *createWorkerPool(Arena *arena) {
  return createWorkerPool(arena, Thread::hardwareConcurrency());
}
