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

template <typename T, size_t N>
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

  operator MutSlice<T>() { return {data, N}; }

  operator Slice<T>() const { return {data, N}; }

  MutSlice<T> asSlice() {
    DCHECK(N <= 0xFFFFFFFF);
    return {data, N};
  }

  Slice<T> asSlice() const { return {data, N}; }

  MutSlice<T> subarray(size_t idxStart) { return asSlice().subarray(idxStart); }

  MutSlice<T> subarray(size_t idxStart, size_t idxEnd) {
    return asSlice().subarray(idxStart, idxEnd);
  }

  Slice<T> subarray(size_t idxStart) const {
    return asSlice().subarray(idxStart);
  }

  Slice<T> subarray(size_t idxStart, size_t idxEnd) const {
    return asSlice().subarray(idxStart, idxEnd);
  }

  MutSlice<T> subarray(Range<size_t> range) {
    return subarray(range.start, range.end);
  }

  MutSlice<T> subarray(Span<size_t> span) { return subarray(rangeFrom(span)); }

  Slice<T> subarray(Range<size_t> range) const {
    return subarray(range.start, range.end);
  }

  Slice<T> subarray(Span<size_t> span) const {
    return subarray(rangeFrom(span));
  }

  T *begin() { return &data[0]; }
  T *end() { return &data[N]; }

  const T *begin() const { return &data[0]; }
  const T *end() const { return &data[N]; }
};
