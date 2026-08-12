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
 * \addtogroup Slice Slice
 * @{
 */

/**
 * \brief Creates a slice initializer from a zero-terminated string constant.
 * Does not include the zero terminator!
 */
#define sliceFromConstChar(s) {(s), sizeof((s)) - 1}
/**
 * \brief Creates a slice initializer from a zero-terminated string constant,
 * including the zero terminator!
 */
#define sliceFromConstCharWithZero(s) {(s), sizeof((s))}
/**
 * \deprecated Prefer `::sliceFrom` from Slice.hpp
 * \private
 */
#define sliceFromArray(arr) {(arr), sizeof((arr)) / sizeof((arr)[0])}

/**
 * \deprecated Prefer `::duplicate` instead
 * \private
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
Slice<char> fromCStrWithZero(const char *s);
/**
 * \brief Creates a slice from a zero-terminated string that doesn't include
 * the zero-terminator.
 *
 * \param s A valid pointer to sequence of zero-terminated
 * characters.
 */
Slice<char> fromCStr(const char *s);


/**
 * \deprecated Prefer `operator==`
 * \private
 */
b32 compareAsString(Slice<char> left, Slice<char> right);

/**
 * \deprecated Prefer Slice<T>::startsWith
 * \private
 */
template <typename T>
bool startsWith(Slice<T> left, Slice<T> prefix) {
  return left.startsWith(prefix);
}

/**
 * \brief Copies `numElements * sizeof(T)` bytes of **memory** from `src` to
 * `dst`.
 */
template <typename T>
T *copyElements(T *dst, const T *src, size_t numElements) {
  return (T *)memcpy(dst, src, numElements * sizeof(T));
}

/**
 * \brief Creates a duplicate of `in`. The duplicated elements are allocated
 * into the provided arena.
 */
template <typename T>
MutSlice<T> duplicate(Arena *arena, Slice<T> in) {
  if (in.empty()) {
    return {nullptr, 0};
  }

  MutSlice<T> ret = {alloc<T>(arena, in.length), in.length};
  copyElements(ret.data, in.data, ret.length);
  return ret;
}

/**
 * \brief Creates a duplicate of `in`. The duplicated elements are allocated
 * into the provided arena.
 */
template <typename T>
MutSlice<T> duplicate(Arena *arena, MutSlice<T> in) {
  return duplicate<T>(arena, static_cast<Slice<T>>(in));
}

/**
 * \brief Creates a new zero-initialized slice with the specified length.
 */
template <typename T>
void alloc(Arena *arena, size_t length, MutSlice<T> &dst) {
  dst.length = length;
  dst.data = alloc<T>(arena, length);
}

/**
 * \brief Creates a new **uninitialized** slice with the specified length.
 */
template <typename T>
void allocNZ(Arena *arena, size_t length, MutSlice<T> &dst) {
  dst.length = length;
  dst.data = allocNZ<T>(arena, length);
}

/**
 * \brief Fills the slice with zeros.
 */
template <typename T>
void zeroMemory(MutSlice<T> s) {
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
[[deprecated]] void copy(MutSlice<T> dst, Slice<T> src) {
  dst.copy(src);
}

/** Takes two slices and returns their concatenation. */
template <typename T>
MutSlice<T> concat(Arena *arena, Slice<T> left, Slice<T> right) {
  size_t lenOut = left.length + right.length;

  MutSlice<T> ret;
  allocNZ(arena, lenOut, ret);

  ret.subarray(0, left.length).copy(left);
  ret.subarray(left.length).copy(right);

  return ret;
}

/**
 * Takes two slices and returns their concatenation with a zero-terminator
 * appended to the end. The resulting slice **includes** the zero-terminator.
 */
template <typename T>
MutSlice<T> concatZeroTerminate(Arena *arena, Slice<T> left, Slice<T> right) {
  size_t lenOut = left.length + right.length + 1;

  MutSlice<T> ret;
  allocNZ(arena, lenOut, ret);

  ret.subarray(0, left.length).copy(left);
  ret.subarray(left.length).copy(right);
  ret[ret.length - 1] = {};

  return ret;
}

/**
 * \brief Creates a zero-terminated copy of the input slice.
 */
template <typename T>
MutSlice<T> zeroTerminated(Arena *arena, Slice<T> in) {
  Slice<T> empty = {};
  return concatZeroTerminate(arena, in, empty);
}

/**
 * \private
 */
template <typename T>
bool endsWith(Slice<T> self, Slice<T> suffix) {
  return self.endsWith(suffix);
}

template <typename T>
MutSlice<T> append(MutSlice<T> dst, Slice<T> src) {
  CHECK(dst.length >= src.length);
  copyElementsInto(dst, src.data, src.length);
  shrinkFromLeftByCount(&dst, src.length);
  return dst;
}

template <typename T>
MutSlice<T> append(MutSlice<T> dst, const T &elem) {
  CHECK(dst.length >= 1);
  copyElementsInto(dst, &elem, 1);
  shrinkFromLeftByCount(&dst, 1);
  return dst;
}

/**
 * \private
 */
template <typename T>
void fill(MutSlice<T> dst, const T &value) {
  dst.fill(value);
}

/**
 * \private
 */
template <typename D, typename S>
void convert(MutSlice<D> dst, Slice<S> src) {
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

/**@}*/
