/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Arena.h"
#include "std/Slice.hpp"

/**
 * @brief Arena and Slice-based helper for the two-call idiom found in Khronos
 * APIs.
 * 
 * @param arena Destination arena
 * @param dst The output slice
 * @param func A function like
 *   `(u32& outputElementCount, T* outputElements) -> R`.
 * @param defaultValue The value to initialize the elements to after
 * allocation.
 *
 * @param Result The result type of the API; non-negative values must represent
 * success. If the API returns void, prefer twoCallV instead.
 */
template <typename Result, typename T, typename F>
Result twoCall(Arena *arena, Slice<T> &dst, F &&func, const T &defaultValue = {}) {
  dst = {nullptr, 0};

  Result rc;
  u32 numElems = 0;
  T *pElems = nullptr;
  // Query element count
  rc = func(numElems, pElems);
  if (rc < 0) {
    return rc;
  }

  // Save the arena so that we can roll back the allocation we're just about to
  // do in case of a failure
  Arena saved = *arena;
  // Allocate space for the elements
  pElems = alloc<T>(arena, numElems);

  // Initialize elements
  for (u32 i = 0; i < numElems; i++) {
    pElems[i] = defaultValue;
  }

  // Fetch elements
  rc = func(numElems, pElems);
  if (rc < 0) {
    // Roll back the arena
    *arena = saved;
    return rc;
  }

  dst = {pElems, numElems};
  return rc;
}

/**
 * @brief Arena and Slice-based helper for the two-call idiom found in Khronos
 * APIs.
 * 
 * @param arena Destination arena
 * @param dst The output slice
 * @param func A function like
 *   `(u32& outputElementCount, T* outputElements) -> void`.
 */
template <typename T, typename F>
void twoCallV(Arena *arena, Slice<T> &dst, F &&func, const T &defaultValue = {}) {
  dst = {nullptr, 0};

  u32 numElems = 0;
  T *pElems = nullptr;
  // Query element count
  func(numElems, pElems);

  // Allocate space for the elements
  pElems = alloc<T>(arena, numElems);

  // Initialize elements
  for (u32 i = 0; i < numElems; i++) {
    pElems[i] = defaultValue;
  }

  // Fetch elements
  func(numElems, pElems);

  dst = {pElems, numElems};
}
