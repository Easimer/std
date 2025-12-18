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

#include <string.h>

#define sliceFromConstChar(s) {(s), sizeof((s)) - 1}
/** @deprecated Prefer {@link sliceFrom} from Slice.hpp */
#define sliceFromArray(arr) {(arr), sizeof((arr)) / sizeof((arr)[0])}

b32 compareAsString(Slice<const char> left, Slice<const char> right);

template <typename T>
bool startsWith(Slice<const T> left, Slice<const T> prefix) {
  if (left.data == nullptr || prefix.data == nullptr) {
    return false;
  }

  if (left.length < prefix.length) {
    return false;
  }

  return memcmp(left.data, prefix.data, prefix.length) == 0;
}

template <typename T>
T *copyElements(T *dst, const T *src, u32 numElements) {
  return (T *)memcpy(dst, src, numElements * sizeof(T));
}

template <typename T>
Slice<T> duplicate(Arena *arena, Slice<const T> in) {
  if (empty(in)) {
    return {nullptr, 0};
  }

  Slice<T> ret = {alloc<T>(arena, in.length), in.length};
  copyElements(ret.data, in.data, ret.length);
  return ret;
}

template <typename T>
Slice<T> duplicate(Arena *arena, Slice<T> in) {
  if (empty(in)) {
    return {nullptr, 0};
  }

  Slice<T> ret = {alloc<T>(arena, in.length), in.length};
  copyElements(ret.data, in.data, ret.length);
  return ret;
}

/**
 * Creates a new zero-initialized slice with the specified length.
 */
template <typename T>
void alloc(Arena *arena, u32 length, Slice<T> &dst) {
  dst.length = length;
  dst.data = alloc<T>(arena, length);
}

/**
 * Creates a new **uninitialized** slice with the specified length.
 */
template <typename T>
void allocNZ(Arena *arena, u32 length, Slice<T> &dst) {
  dst.length = length;
  dst.data = allocNZ<T>(arena, length);
}

template <typename T>
void zeroMemory(Slice<T> s) {
  memset(s.data, 0, s.length * sizeof(T));
}

template <typename T>
void copy(Slice<T> dst, Slice<const T> src) {
  if (empty(src)) {
    return;
  }

  CHECK(src.length <= dst.length);
  copyElements(dst.data, src.data, src.length);
}

template <typename T>
void copy(Slice<T> dst, Slice<T> src) {
  copy(dst, src.asConst());
}

template <typename T>
Slice<u8> makeSlice(Arena *arena, const T *src, u32 len) {
  T *newBuf = alloc<T>(arena, len);
  copyElements(newBuf, src, len);
  return {newBuf, len};
}

Slice<const char> fromCStr(const char *s);

/** Takes two slices and returns their concatenation. */
template <typename T>
Slice<T> concat(Arena *arena, Slice<const T> left, Slice<const T> right) {
  u32 lenOut = left.length + right.length;

  Slice<T> ret;
  allocNZ(arena, lenOut, ret);

  copy(subarray(ret, 0, left.length), left);
  copy(subarray(ret, left.length), right);

  return ret;
}

/**
 * Takes two slices and returns their concatenation with a null-terminator
 * appended to the end.
 */
template <typename T>
Slice<T> concatZeroTerminate(Arena *arena,
                             Slice<const T> left,
                             Slice<const T> right) {
  u32 lenOut = left.length + right.length + 1;

  Slice<T> ret;
  allocNZ(arena, lenOut, ret);

  copy(subarray(ret, 0, left.length), left);
  copy(subarray(ret, left.length), right);
  ret[ret.length - 1] = {};

  return ret;
}

template <typename T>
bool operator==(Slice<const T> lhs, Slice<const T> rhs) {
  if (lhs.length != rhs.length) {
    return false;
  }

  for (u32 i = 0; i < lhs.length; i++) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }

  return true;
}

template <typename T>
bool endsWith(Slice<const T> self, Slice<const T> suffix) {
  if (self.length < suffix.length) {
    return false;
  }

  Slice<const T> end = subarray(self, self.length - suffix.length);
  return end == suffix;
}

template <typename T>
Slice<T> append(Slice<T> dst, Slice<T> src) {
  CHECK(dst.length >= src.length);
  copyElementsInto(dst, src.data, src.length);
  shrinkFromLeftByCount(&dst, src.length);
  return dst;
}

template <typename T>
Slice<T> append(Slice<T> dst, Slice<const T> src) {
  CHECK(dst.length >= src.length);
  copyElementsInto(dst, src.data, src.length);
  shrinkFromLeftByCount(&dst, src.length);
  return dst;
}

template <typename T>
Slice<T> append(Slice<T> dst, const T &elem) {
  CHECK(dst.length >= 1);
  copyElementsInto(dst, &elem, 1);
  shrinkFromLeftByCount(&dst, 1);
  return dst;
}

template <typename T>
void fill(Slice<T> dst, const T &value) {
  for (u32 i = 0; i < dst.length; i++) {
    dst[i] = value;
  }
}

template <typename D, typename S>
void convert(Slice<D> dst, Slice<const S> src) {
  DCHECK(dst.length == src.length);

  for (u32 i = 0; i < dst.length; i++) {
    dst[i] = D(src[i]);
  }
}

template <typename T, typename F>
T *findFirst(Slice<T> self, F &&filter) {
  for (auto [v, _] : self) {
    if (filter(v)) {
      return &v;
    }
  }

  return nullptr;
}
