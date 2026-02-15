/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "./Array.hpp"
#include "./Check.h"
#include "./Types.h"

#if EMSCRIPTEN
#include <emscripten/atomic.h>
#include <emscripten/threading.h>
#else
#include <atomic>
#endif

namespace impl {
#if EMSCRIPTEN
struct AtomicU32 {
  alignas(4) _Atomic u32 v;

  u32 load() { return emscripten_atomic_load_u32(&v); }
  void store(u32 value) { emscripten_atomic_store_u32(&v, value); }
  void notifyOne() { emscripten_atomic_notify(&v, 1); }
  void notifyAll() {
    emscripten_atomic_notify(&v, EMSCRIPTEN_NOTIFY_ALL_WAITERS);
  }
  void wait(u32 old) {
    emscripten_atomic_wait_u32(&v, old, ATOMICS_WAIT_DURATION_INFINITE);
  }
  bool compareExchange(u32 expected, u32 newValue) {
    u32 old = emscripten_atomic_cas_u32(&v, expected, newValue);
    return old == expected;
  }
  u32 fetchAdd(u32 a) { return emscripten_atomic_add_u32(&v, a); }
  u32 fetchSub(u32 a) { return emscripten_atomic_sub_u32(&v, a); }
};
#else
struct AtomicU32 {
  std::atomic<u32> v;

  u32 load() { return v.load(); }
  void store(u32 value) { v.store(value); }
  void notifyOne() { v.notify_one(); }
  void notifyAll() { v.notify_all(); }

  void wait(u32 old) { v.wait(old); }

  bool compareExchange(u32 expected, u32 newValue) {
    return v.compare_exchange_strong(expected, newValue);
  }

  u32 fetchAdd(u32 a) { return v.fetch_add(a); }
  u32 fetchSub(u32 a) { return v.fetch_sub(a); }
};
#endif

static u32 eytzingerParent(u32 i) {
  return (i - 1) / 2;
}

static u32 eytzingerLeft(u32 i) {
  return 2 * i + 1;
}

static u32 eytzingerRight(u32 i) {
  return 2 * i + 2;
}

}  // namespace impl

template <u32 Power>
struct SignalTree {
  struct Node {
    alignas(64) impl::AtomicU32 counter;
    impl::AtomicU32 priority;
  };

  static constexpr u32 NumLevels = Power + 1;
  static constexpr u32 NumTotalNodes = (1 << (Power + 1)) - 1;
  static constexpr u32 NodeIndexMask = (1 << Power) - 1;
  static constexpr u32 NumLeafNodes = 1 << Power;
  static constexpr u32 IdxFirstLeafNode = (1 << Power) - 1;

  Array<Node, NumTotalNodes> nodes;

  impl::AtomicU32 numThreadsWaiting;

  void notifyOne() {
    if (numThreadsWaiting.load() != 0) {
      nodes[0].counter.notifyOne();
    }
  }

  void notifyAll() {
    if (numThreadsWaiting.load() != 0) {
      nodes[0].counter.notifyAll();
    }
  }

  void setSignalForJob(u32 idxJob, bool priority) {
    DCHECK(idxJob < NumLeafNodes);
    u32 idxNode = IdxFirstLeafNode + idxJob;
    DCHECK(idxNode < NumTotalNodes);

    for (u32 idxLevel = 0; idxLevel < NumLevels; idxLevel++) {
      DCHECK(idxNode < NumTotalNodes);
      DCHECK((idxNode == 0 && idxLevel == NumLevels - 1) ||
             idxLevel != NumLevels - 1);
      nodes[idxNode].counter.fetchAdd(1);
      if (priority) {
        nodes[idxNode].priority.fetchAdd(1);
      }
      idxNode = impl::eytzingerParent(idxNode);
    }
  }

  void clearPriorityForJob(u32 idxJob) {
    DCHECK(idxJob < NumLeafNodes);
    u32 idxNode = IdxFirstLeafNode + idxJob;
    DCHECK(idxNode < NumTotalNodes);

    for (u32 idxLevel = 0; idxLevel < NumLevels; idxLevel++) {
      DCHECK(idxNode < NumTotalNodes);
      DCHECK((idxNode == 0 && idxLevel == NumLevels - 1) ||
             idxLevel != NumLevels - 1);
      nodes[idxNode].priority.fetchSub(1);
      idxNode = impl::eytzingerParent(idxNode);
    }
  }

  bool isLeafNode(u32 idxNode) const { return idxNode >= IdxFirstLeafNode; }
  u32 numLevels() const { return NumLevels; }

  void waitForRootToChange() {
    numThreadsWaiting.fetchAdd(1);
    nodes[0].counter.wait(0);
    numThreadsWaiting.fetchSub(1);
  }

  void setRoot(u32 value) {
    nodes[0].counter.store(value);
    nodes[0].counter.notifyAll();
  }

  bool tryDecrementNode(u32 idxNode) {
    impl::AtomicU32 *counter;
    u32 count;
    do {
      counter = &nodes[idxNode].counter;
      count = counter->load();
      if (count == 0) {
        return false;
      }
    } while (!counter->compareExchange(count, count - 1));
    return true;
  }

  bool tryDecrementPriority(u32 idxNode,
                            u32 idxLeft,
                            u32 idxRight,
                            u32 &idxTaken) {
    if (nodes[idxNode].priority.load() == 0) {
      return false;
    }

    if (nodes[idxLeft].priority.load() > 0) {
      if (tryDecrementNode(idxLeft)) {
        idxTaken = idxLeft;
        return true;
      }
    } else if (nodes[idxRight].priority.load() > 0) {
      if (tryDecrementNode(idxRight)) {
        idxTaken = idxRight;
        return true;
      }
    }

    return false;
  }
};
