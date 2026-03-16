/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Arena.h"
#include "std/Slice.hpp"
#include "std/Vector.hpp"
#include "std/VectorUtils.hpp"

template <typename T>
Slice<Slice<T>> split(Arena *arena, Slice<T> arr, const T &sep) {
  if (arr.empty()) {
    return {};
  }

  Arena::Scope temp = getScratch(&arena, 1);
  Vector<Slice<T>> ret;

  u32 idxPrevStart = 0;
  for (auto [elem, i] : arr) {
    if (elem == sep) {
      Slice<T> s = arr.subarray(idxPrevStart, i);
      appendVal(temp, &ret, s);
      idxPrevStart = i + 1;
    }
  }

  if (idxPrevStart <= arr.length) {
    Slice<T> s = arr.subarray(idxPrevStart);
    appendVal(temp, &ret, s);
  }

  return copyToSlice(arena, ret);
}

template <typename T>
Slice<Slice<const T>> split(Arena *arena, Slice<const T> arr, const T &sep) {
  if (arr.empty()) {
    return {};
  }

  Arena::Scope temp = getScratch(&arena, 1);
  Vector<Slice<const T>> ret;

  u32 idxPrevStart = 0;
  for (auto [elem, i] : arr) {
    if (elem == sep) {
      Slice<const T> s = arr.subarray(idxPrevStart, i);
      appendVal(temp, &ret, s);
      idxPrevStart = i + 1;
    }
  }

  if (idxPrevStart <= arr.length) {
    Slice<const T> s = arr.subarray(idxPrevStart);
    appendVal(temp, &ret, s);
  }

  return copyToSlice(arena, ret);
}
