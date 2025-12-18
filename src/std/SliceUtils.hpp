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

/** @file SliceUtils.hpp */

/**
 * \defgroup SliceCreation Slice creation
 * @{
 */

#define sliceFromConstChar(s) {(s), sizeof((s)) - 1}
#define sliceFromConstCharWithZero(s) {(s), sizeof((s))}
/** @deprecated Prefer \ref sliceFrom from Slice.hpp */
#define sliceFromArray(arr) {(arr), sizeof((arr)) / sizeof((arr)[0])}

/**
 * \deprecated Prefer \ref "duplicate" instead
 */
template <typename T>
Slice<T> makeSlice(Arena *arena, const T *src, u32 len) {
  T *newBuf = alloc<T>(arena, len);
  copyElements(newBuf, src, len);
  return {newBuf, len};
}

/**
 * \brief Creates a slice from a zero-terminated string that **also** includes
 * the zero-terminator.
 *
 * \param s A valid pointer to sequence of zero-terminated
 * characters.
 */
Slice<const char> fromCStrWithZero(const char *s);
/**
 * \brief Creates a slice from a zero-terminated string that doesn't include
 * the zero-terminator.
 *
 * \param s A valid pointer to sequence of zero-terminated
 * characters.
 */
Slice<const char> fromCStr(const char *s);


/**
 * @deprecated Prefer `operator==`
 */
b32 compareAsString(Slice<const char> left, Slice<const char> right);

/**
 * @deprecated Prefer Slice<T>::startsWith
 */
template <typename T>
bool startsWith(Slice<const T> left, Slice<const T> prefix) {
  return left.startsWith(prefix);
}

/**
 * \brief Copies `numElements * sizeof(T)` bytes of **memory** from `src` to
 * `dst`.
 */
template <typename T>
T *copyElements(T *dst, const T *src, u32 numElements) {
  return (T *)memcpy(dst, src, numElements * sizeof(T));
}

/**
 * \brief Creates a duplicate of `in`. The duplicated elements are allocated
 * into the provided arena.
 */
template <typename T>
Slice<T> duplicate(Arena *arena, Slice<const T> in) {
  if (empty(in)) {
    return {nullptr, 0};
  }

  Slice<T> ret = {alloc<T>(arena, in.length), in.length};
  copyElements(ret.data, in.data, ret.length);
  return ret;
}

/**
 * \brief Creates a duplicate of `in`. The duplicated elements are allocated
 * into the provided arena.
 */
template <typename T>
Slice<T> duplicate(Arena *arena, Slice<T> in) {
  return duplicate(arena, in.asConst());
}

/**
 * \brief Creates a new zero-initialized slice with the specified length.
 */
template <typename T>
void alloc(Arena *arena, u32 length, Slice<T> &dst) {
  dst.length = length;
  dst.data = alloc<T>(arena, length);
}

/**
 * \brief Creates a new **uninitialized** slice with the specified length.
 */
template <typename T>
void allocNZ(Arena *arena, u32 length, Slice<T> &dst) {
  dst.length = length;
  dst.data = allocNZ<T>(arena, length);
}

/**
 * \brief Fills the slice with zeros.
 */
template <typename T>
void zeroMemory(Slice<T> s) {
  if (s.data == nullptr) {
    return;
  }

  memset(s.data, 0, s.length * sizeof(T));
}

/**
 * \brief Copies all elements from `src` to the beginning of `dst`. The
 * destination slice must be at least as big as the source slice.
 */
template <typename T>
void copy(Slice<T> dst, Slice<const T> src) {
  if (empty(src)) {
    return;
  }

  CHECK(src.length <= dst.length);
  for (u32 i = 0; i < src.length; i++) {
    dst[i] = src[i];
  }
}

/**
 * \brief Copies all elements from `src` to the beginning of `dst`. The
 * destination slice must be at least as big as the source slice.
 */
template <typename T>
void copy(Slice<T> dst, Slice<T> src) {
  copy(dst, src.asConst());
}

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
 * Takes two slices and returns their concatenation with a zero-terminator
 * appended to the end. The resulting slice **includes** the zero-terminator.
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
bool endsWith(Slice<const T> self, Slice<const T> suffix) {
  return self.endsWith(suffix);
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
  dst.fill(value);
}

template <typename D, typename S>
void convert(Slice<D> dst, Slice<const S> src) {
  dst.copyWithConversionTo(dst);
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
