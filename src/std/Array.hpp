/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Check.h"
#include "std/Slice.hpp"
#include "std/Types.h"

#include <initializer_list>

template<typename T, size_t N>
struct Array {
  static constexpr size_t length = N;

  T data[N];

  constexpr Array() : data() {}
  constexpr Array(std::initializer_list<T> l) : data() {
    if (l.size() > N) {
      throw "Too many elements";
    }

    size_t index = 0;
    for (const T &item : l) {
      if (index < N) {
        data[index++] = item;
      }
    }
  }

  Array(const Array<T, N> &other) : data() {
    for (size_t i = 0; i < N; i++) {
      data[i] = other.data[i];
    }
  }

  T &operator[](size_t idxElem) { return data[idxElem]; }
  const T &operator[](size_t idxElem) const { return data[idxElem]; }

  operator Slice<T>() {
    DCHECK(N <= 0xFFFFFFFF);
    return {data, u32(N)};
  }

  operator Slice<const T>() const {
    DCHECK(N <= 0xFFFFFFFF);
    return {data, u32(N)};
  }

  Slice<T> asSlice() {
    DCHECK(N <= 0xFFFFFFFF);
    return {data, u32(N)};
  }

  Slice<const T> asSlice() const {
    DCHECK(N <= 0xFFFFFFFF);
    return {data, u32(N)};
  }

  Slice<T> subarray(u32 idxStart) {
    Slice<T> whole = (*this);
    return whole.subarray(idxStart);
  }

  Slice<T> subarray(u32 idxStart, u32 idxEnd) {
    Slice<T> whole = (*this);
    return whole.subarray(idxStart, idxEnd);
  }

  Slice<const T> subarray(u32 idxStart) const {
    Slice<const T> whole = (*this);
    return whole.subarray(idxStart);
  }

  Slice<const T> subarray(u32 idxStart, u32 idxEnd) const {
    Slice<const T> whole = (*this);
    return whole.subarray(idxStart, idxEnd);
  }

  Slice<T> subarray(Range<u32> range) {
    return subarray(range.start, range.end);
  }

  Slice<T> subarray(Span<u32> span) { return subarray(rangeFrom(span)); }

  Slice<const T> subarray(Range<u32> range) const {
    return subarray(range.start, range.end);
  }

  Slice<const T> subarray(Span<u32> span) const {
    return subarray(rangeFrom(span));
  }

  T *begin() { return &data[0]; }
  T *end() { return &data[N]; }
};
