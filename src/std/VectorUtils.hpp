/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"
#include "std/Vector.hpp"

/** @file VectorUtils.hpp */

/**
 * \addtogroup Vector Vector
 * @{
 */

template <typename T>
Vector<T> makeVectorFrom(Arena *arena, Slice<T> s) {
  Vector<T> ret = vectorWithInitialCapacity<T>(arena, s.length);
  copyElements(append(arena, &ret, s.length), s.data, s.length);
  return ret;
}

/**
 * Copies the contents of the vector into a slice. The storage for the slice
 * will be allocated into the provided arena.
 */
template <typename T>
Slice<T> copyToSlice(Arena *arena, Vector<T> src) {
  if (src.data == nullptr || src.length == 0) {
    return {nullptr, 0};
  }
  T *newData = alloc<T>(arena, src.length);
  copyElements(newData, src.data, src.length);
  Slice<T> ret = {newData, src.length};
  return ret;
}

template <typename T>
Slice<T> sliceFrom(Vector<T> src) {
  return {src.data, src.length};
}

/**@}*/
