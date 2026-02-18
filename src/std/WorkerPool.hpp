/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Arena.h"
#include "std/Optional.hpp"
#include "std/Types.h"

struct WorkerPool;
struct WorkContract;

struct ThreadIndex {
  u32 x, y, z;
};

struct Dispatch {
  void *parameters;
  ThreadIndex threadIndex;

  u32 idxPhysicalThread;
  WorkerPool *workerPool;

  template <typename T>
  T &parametersAs() const {
    return *reinterpret_cast<T *>(parameters);
  }
};

using KernelEntryPoint = void (*)(const Dispatch *dispatch);

enum WorkContractFlags : u32 {
  /**
   * Mark this work contract as high priority.
   */
  WC_HIGH_PRIORITY = 1 << 0,

  /**
   * Internal. This work contract is owned by the job system itself.
   */
  INTERNAL_WC_OWNED = 1u << 31,
};

struct WorkerPool {
  virtual void shutdown() = 0;

  virtual WorkContract *createWorkContract(Arena *arena,
                                           KernelEntryPoint entry,
                                           u32 flags = 0) = 0;
  virtual void dispatch(WorkContract *workContract,
                        void *parameters,
                        u32 numThreadsX = 1,
                        u32 numThreadsY = 1,
                        u32 numThreadsZ = 1) = 0;
  virtual void release(WorkContract *workContract) = 0;
};

struct WorkerPoolWorkerInitializer {
  KernelEntryPoint func;
  void *parameters;
};

struct WorkerPoolCreateInfo {
  Optional<u32> numThreads;
  Optional<WorkerPoolWorkerInitializer> workerInitializer;
};

WorkerPool *createWorkerPool(Arena *arena);
WorkerPool *createWorkerPool(Arena *arena, u32 numThreads);
WorkerPool *createWorkerPool(Arena *arena,
                             const WorkerPoolCreateInfo &createInfo);


