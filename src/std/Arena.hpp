/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "./Allocator.hpp"
#include "./Check.h"
#include "./Modules.h"
#include "./Types.h"

struct Arena;

struct Arena final : sn::Allocator {
  u8 *beg;
  u8 *end;

  struct Scope;

  Arena() : beg(nullptr), end(nullptr) {}
  Arena(u8 *beg, u8 *end) : beg(beg), end(end) {}

  u8 *alloc(size_t byteLength, size_t alignment) noexcept override;
  bool resize(u8 *allocation,
              size_t alignment,
              size_t nextLength) noexcept override;
  u8 *remap(u8 *allocation,
            size_t alignment,
            size_t nextLength) noexcept override;
  void free(u8 *allocation, size_t alignment) noexcept override;

  void restoreFrom(const Arena &saved) noexcept;

  SN_STD_API static void setAllocatorsForThisThread(Arena *arena0,
                                                    Arena *arena1);
  /**
   * Finds an arena that is distinct from the provided arena. If it is one of
   * the arenas associated with this thread, then the function will return the
   * other one.
   */
  SN_STD_API static Arena *getScratch(Arena *conflict);
  /**
   * Returns one of the arenas associated with this thread.
   */
  SN_STD_API static Arena *getScratch();
};

/**
 * \brief Allocates zero-initialized space from an arena.
 *
 * \param arena Target arena
 * \param sizObject Size of the objects
 * \param sizAlign Required alignment
 * \param numObjects Number of objects
 *
 * \returns A valid pointer to an zero-initialized allocation that is aligned
 * and has capacity to store objects of the specified size and count.
 */
u8 *alloc(Arena *arena, size_t sizObject, size_t sizAlign, size_t numObjects);
/**
 * \brief Allocates uninitialized space from an arena. Prefer `alloc` unless the
 * allocated space is going to be initialized or overwritten anyways.
 *
 * \param arena Target arena
 * \param sizObject Size of the objects
 * \param sizAlign Required alignment
 * \param numObjects Number of objects
 *
 * \returns A valid pointer to an uninitialized allocation that is aligned
 * and has capacity to store objects of the specified size and count.
 */
u8 *allocNZ(Arena *arena, size_t sizObject, size_t sizAlign, size_t numObjects);

template <typename T, size_t Align>
T *alloc(Arena *a, size_t count = 1) {
  void *ptr = alloc(a, sizeof(T), Align, count);
  return reinterpret_cast<T *>(SN_ASSUME_ALIGNED(ptr, Align));
}

template <typename T, size_t Align>
T *allocNZ(Arena *a, size_t count = 1) {
  void *ptr = allocNZ(a, sizeof(T), Align, count);
  return reinterpret_cast<T *>(SN_ASSUME_ALIGNED(ptr, Align));
}

template <typename T>
T *alloc(Arena *a, size_t count = 1) {
  return alloc<T, alignof(T)>(a, count);
}

template <typename T>
T *allocNZ(Arena *a, size_t count = 1) {
  return allocNZ<T, alignof(T)>(a, count);
}

/**
 * \brief Saves the current state of an Arena and restores it when its lifetime
 * ends. This can be used to automatically free temporary arena allocations at
 * the end of a scope. Can be implicitly cast to an `Arena*` thus it can be
 * passed to anyone who accepts one of those.
 */
struct Arena::Scope {
  Arena *arena;
  Arena saved;

  Scope() : Scope(Arena::getScratch()) {}
  Scope(Arena *arena) : arena(arena), saved(*arena) {}
  ~Scope() { reset(); }
  Scope(const Scope &) = delete;
  Scope(Scope &&) = delete;
  void operator=(const Scope &) = delete;
  void operator=(Scope &&) = delete;
  operator Arena *() const noexcept { return arena; }

  void reset() const noexcept { arena->restoreFrom(saved); }

  [[deprecated]] Scope *operator->() { return this; }
};
