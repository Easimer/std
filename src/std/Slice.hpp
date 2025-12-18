/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Check.h"
#include "std/Types.h"

/** A span with a start index and the number of elements. */
template <typename T>
struct Span {
  T start;
  T count;
};

/** A half-open range with an inclusive start and an exclusive end index. */
template <typename T>
struct Range {
  T start;
  T end;
};

template <typename T>
Span<T> spanFrom(Range<T> r) {
  T start = r.start;
  T count = r.end - r.start;
  return Span<T>{start, count};
}

template <typename T>
Range<T> rangeFrom(Span<T> s) {
  T start = s.start;
  T end = s.start + s.count;
  return Range<T>{start, end};
}

template <typename T>
struct SliceLegacyIterator {
  using Value = T;

  struct Element {
    T &value;
    u32 index;
  };

  T *const data;
  u32 index;

  bool operator!=(const SliceLegacyIterator<T> &other) const {
    return index != other.index;
  }

  void operator++() { index++; }

  Element operator*() { return {data[index], index}; }
};

template <typename T>
using SliceIterator = SliceLegacyIterator<T>;

/**
 * A view on a section of a homogeneous array. The data being viewed is not
 * owned by the slice.
 *
 * This can be iterated with a for-each expression; the iterator yields
 * `[value, index]` pairs.
 */
template <typename T>
struct Slice {
  using KeyT = u32;
  using ValueT = T;

  T *data = nullptr;
  u32 length = 0;

  T &operator[](u32 i) const {
    DCHECK(data != nullptr);
    DCHECK(i < length);
    return data[i];
  }

  T &operator[](i32 i) const {
    DCHECK(data != nullptr);
    DCHECK(0 <= i);
    DCHECK(i < length);
    return data[i];
  }

  SliceLegacyIterator<T> begin() { return {data, 0}; }
  SliceLegacyIterator<T> end() { return {data, length}; }

  SliceLegacyIterator<const T> begin() const { return {data, 0}; }
  SliceLegacyIterator<const T> end() const { return {data, length}; }

  Slice<const T> asConst() const { return {data, length}; }

  bool operator==(Slice<const T> other) const {
    if (length != other.length) {
      return false;
    }

    for (u32 i = 0; i < length; i++) {
      if (data[i] != other[i]) {
        return false;
      }
    }

    return true;
  }
};

/**
 * Steps the slice forward by N elements and decreases its length accordingly.
 * The slice must not have atleast N elements.
 */
template <typename T>
inline void shrinkFromLeftByCount(Slice<T> *target, u32 numElements) {
  CHECK(target->data != NULL);
  CHECK(target->length >= numElements);
  target->data += numElements;
  target->length -= numElements;
}

template <typename T>
inline b32 indexOf(Slice<const T> s, const T &needle, u32 *out) {
  if (empty(s)) {
    return false;
  }

  for (u32 i = 0; i < s.length; i++) {
    if (s[i] == needle) {
      *out = i;
      return true;
    }
  }

  return false;
}

template <typename T>
inline b32 indexOf(Slice<T> s, const T &needle, u32 *out) {
  return indexOf(s.asConst(), needle, out);
}

template <typename T>
inline b32 lastIndexOf(Slice<T> s, const T &needle, u32 *out) {
  if (empty(s)) {
    return false;
  }

  // NOTE(danielm): condition becomes false after `i` underflows
  for (u32 i = s.length - 1; i < s.length; i--) {
    if (s[i] == needle) {
      *out = i;
      return true;
    }
  }

  return false;
}

template <typename T>
inline Slice<T> subarray(Slice<T> s, u32 idxStart, u32 idxEnd) {
  if (idxEnd <= idxStart || s.length <= idxStart) {
    return {nullptr, 0};
  }

  if (s.length < idxEnd) {
    idxEnd = s.length;
  }

  u32 len = idxEnd - idxStart;
  T *data = s.data + idxStart;

  return {data, len};
}

template <typename T>
inline Slice<T> subarray(Slice<T> s, Range<u32> range) {
  return subarray(s, range.start, range.end);
}

template <typename T>
inline Slice<T> subarray(Slice<T> s, Span<u32> span) {
  return subarray(s, rangeFrom(span));
}

template <typename T>
inline Slice<T> subarray(Slice<T> s, u32 idxStart) {
  return subarray(s, idxStart, s.length);
}

/**
 * Steps the slice forward by one element and decreases its length.
 * The slice must not be empty.
 */
template <typename T>
inline void shrinkFromLeft(Slice<T> *target) {
  CHECK(target->data != NULL);
  CHECK(target->length != 0);
  target->data++;
  target->length -= 1;
}

template <typename T>
inline b32 empty(Slice<T> s) {
  return s.length == 0;
}

/**
 * A macro that can be used to supply a slice (usually a Slice<char>) as an
 * argument to a printf-style function when using a directive like "%.*s".
 */
#define FMT_SLICE(s) (s).length, (s).data

/**
 * Casts a slice from one type to another.
 */
template <typename D, typename S>
Slice<D> cast(Slice<S> in) {
  static_assert(sizeof(D) < sizeof(S) || (sizeof(D) % sizeof(S)) == 0);
  static_assert(sizeof(S) < sizeof(D) || (sizeof(S) % sizeof(D)) == 0);
  Slice<D> ret;
  ret.data = (D *)in.data;
  ret.length = in.length * sizeof(S) / sizeof(D);
  return ret;
}

template <typename T>
inline void copyElementsInto(Slice<T> s,
                             const T *src,
                             u32 numElements,
                             u32 offset = 0) {
  if (numElements == 0) {
    return;
  }
  CHECK(offset + numElements <= s.length);
  if (offset >= s.length) {
    return;
  }

  T *dst = s.data + offset;
  for (u32 i = 0; i < numElements; i++) {
    dst[i] = src[i];
  }
}

template <typename T>
inline u64 byteLength(Slice<T> s) {
  return s.length * sizeof(T);
}

template <typename T, size_t N>
Slice<T> sliceFrom(T (&p)[N]) {
  return {p, N};
}

template <typename T, size_t N>
constexpr Slice<const T> sliceFrom(const T (&p)[N]) {
  return {p, N};
}

template <typename T>
bool contains(Slice<T> s, const T &needle) {
  u32 discard;
  return indexOf(s, needle, &discard);
}

template <typename T>
bool contains(Slice<const T> s, const T &needle) {
  u32 discard;
  return indexOf(s, needle, &discard);
}

template <typename T, typename F>
bool all(Slice<T> list, F &&condition) {
  for (auto [elem, _] : list) {
    if (!condition(elem)) {
      return false;
    }
  }

  return true;
}

template <typename T, typename F>
bool any(Slice<T> list, F &&condition, u32 &index) {
  for (auto [elem, idxElem] : list) {
    if (condition(elem)) {
      index = idxElem;
      return true;
    }
  }

  return false;
}

template <typename T, typename F>
bool any(Slice<T> list, F &&condition) {
  u32 i;
  return any(list, condition, i);
}

template <typename T>
void reverse(Slice<T> s) {
  const u32 idxLast = s.length - 1;
  const u32 idxMid = s.length / 2;
  for (u32 idxCur = 0; idxCur < idxMid; idxCur++) {
    u32 idxMirror = idxLast - idxCur;
    T t = s[idxCur];
    s[idxCur] = s[idxMirror];
    s[idxMirror] = t;
  }
}

/**
 * Create a mutable slice from an STL container.
 */
template <typename C>
Slice<typename C::value_type> mutSliceFromStd(C &container) {
  return {container.data(), (u32)container.size()};
}

/**
 * Create a slice from an STL container.
 */
template <typename C>
Slice<const typename C::value_type> sliceFromStd(const C &container) {
  return {container.data(), (u32)container.size()};
}
