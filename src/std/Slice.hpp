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

/**
 * \brief A span with a start index and the number of elements.
 */
template <typename T>
struct Span {
  /** \brief Index of the first element. */
  T start;
  /** \brief The number of elements */
  T count;
};

/**
 * \brief A half-open range with an inclusive start and an exclusive end index.
 */
template <typename T>
struct Range {
  /** \brief Index of the first element. */
  T start;
  /** \brief Index of the end; not included. */
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

  /**
   * \brief Checks equality with an other slice.
   *
   * Elements are compared with the equality operator.
   */
  bool operator==(Slice<T> other) const {
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

  bool empty() const { return length == 0; }
  u64 byteLength() const { return length * u64(sizeof(T)); }

  /**
   * \brief Casts a slice to another type.
   */
  template <typename D>
  Slice<D> cast() const {
    static_assert(sizeof(D) < sizeof(T) || (sizeof(D) % sizeof(T)) == 0);
    static_assert(sizeof(T) < sizeof(D) || (sizeof(T) % sizeof(D)) == 0);
    Slice<D> ret;
    ret.data = (D *)data;
    ret.length = length * sizeof(T) / sizeof(D);
    return ret;
  }

  bool indexOf(const T &needle, u32 *out) const {
    if (empty()) {
      return false;
    }

    for (u32 i = 0; i < length; i++) {
      if ((*this)[i] == needle) {
        *out = i;
        return true;
      }
    }

    return false;
  }

  bool lastIndexOf(const T &needle, u32 *out) const {
    if (empty()) {
      return false;
    }

    // NOTE(danielm): condition becomes false after `i` underflows
    for (u32 i = length - 1; i < length; i--) {
      if ((*this)[i] == needle) {
        *out = i;
        return true;
      }
    }

    return false;
  }

  bool contains(const T &needle) const {
    u32 discard;
    return indexOf(needle, &discard);
  }

  Slice<T> subarray(u32 idxStart, u32 idxEnd) const {
    if (idxEnd <= idxStart || length <= idxStart) {
      return {nullptr, 0};
    }

    if (length < idxEnd) {
      idxEnd = length;
    }

    u32 len = idxEnd - idxStart;
    T *start = this->data + idxStart;

    return {start, len};
  }

  Slice<T> subarray(Range<u32> range) const {
    return subarray(range.start, range.end);
  }

  Slice<T> subarray(Span<u32> span) const {
    return subarray(rangeFrom(span));
  }

  Slice<T> subarray(u32 idxStart) const { return subarray(idxStart, length); }

  /**
   * \brief Steps the slice forward by N elements and decreases its length
   * accordingly. The slice must have at least `numElements` elements.
   */
  void shrinkFromLeftByCount(u32 numElements) {
    CHECK(data != nullptr);
    CHECK(length >= numElements);
    data += numElements;
    length -= numElements;
  }

  /**
   * \brief Steps the slice forward by one element and decreases its length.
   * The slice must not be empty.
   */
  void shrinkFromLeft() { shrinkFromLeftByCount(1); }

  /**
   * \brief Copies all elements from `source` into the beginning of this slice.
   */
  void copy(Slice<const T> source) const {
    if (empty(source)) {
      return;
    }

    CHECK(source.length <= length);
    for (u32 i = 0; i < source.length; i++) {
      (*this)[i] = source[i];
    }
  }

  /**
   * \brief Tests whether every element of this slice satisfies `condition`.
   * \param condition A callable object that takes in `T&` and returns a
   * boolean.
   */
  template <typename F>
  bool all(F &&condition) const {
    for (auto [elem, _] : (*this)) {
      if (!condition(elem)) {
        return false;
      }
    }

    return true;
  }

  /**
   * \brief Tests whether any element of this slice satisfies `condition`.
   * \param condition A callable object that takes in `T&` and returns a
   * boolean.
   * \param index A reference to a u32 that will receive the index of the first
   * such element.
   */
  template <typename F>
  bool any(F &&condition, u32 &index) const {
    for (auto [elem, idxElem] : (*this)) {
      if (condition(elem)) {
        index = idxElem;
        return true;
      }
    }

    return false;
  }

  /**
   * \brief Tests whether any element of this slice satisfies `condition`.
   * \param condition A callable object that takes in `T&` and returns a
   * boolean.
   */
  template <typename F>
  bool any(F &&condition) const {
    u32 discard;
    return any(condition, discard);
  }

  /**
   * \brief Reverses the contents of this slice.
   */
  void reverse() {
    Slice<T> &s = *this;
    const u32 idxLast = s.length - 1;
    const u32 idxMid = s.length / 2;
    for (u32 idxCur = 0; idxCur < idxMid; idxCur++) {
      u32 idxMirror = idxLast - idxCur;
      T t = s[idxCur];
      s[idxCur] = s[idxMirror];
      s[idxMirror] = t;
    }
  }

  bool startsWith(Slice<const T> prefix) const {
    if (prefix.empty()) {
      return true;
    }

    if (length < prefix.length) {
      return false;
    }

    return subarray(0, prefix.length).asConst() == prefix;
  }

  bool endsWith(Slice<const T> suffix) const {
    if (suffix.empty()) {
      return true;
    }

    if (length < suffix.length) {
      return false;
    }

    return subarray(length - suffix.length).asConst() == suffix;
  }

  void fill(const T &value) {
    for (u32 i = 0; i < length; i++) {
      (*this)[i] = value;
    }
  }

  template <typename D>
  Slice<D> copyWithConversionTo(Slice<D> dst) const {
    DCHECK(dst.length == length);

    for (u32 i = 0; i < dst.length; i++) {
      const T &src = (*this)[i];
      dst[i] = D(src);
    }

    return dst;
  }
};

/**
 * \deprecated Prefer Slice<T>::shrinkFromLeftByCount
 */
template <typename T>
inline void shrinkFromLeftByCount(Slice<T> *target, u32 numElements) {
  target->shrinkFromLeftByCount(numElements);
}

/**
 * \deprecated Prefer Slice<T>::indexOf
 */
template <typename T>
inline b32 indexOf(Slice<const T> s, const T &needle, u32 *out) {
  return s.indexOf(needle, out) ? b32(1) : b32(0);
}

/**
 * \deprecated Prefer Slice<T>::indexOf
 */
template <typename T>
inline b32 indexOf(Slice<T> s, const T &needle, u32 *out) {
  return indexOf(s.asConst(), needle, out);
}

/**
 * \deprecated Prefer Slice<T>::lastIndexOf
 */
template <typename T>
inline b32 lastIndexOf(Slice<T> s, const T &needle, u32 *out) {
  return s.lastIndexOf(needle, out);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 */
template <typename T>
inline Slice<T> subarray(Slice<T> s, u32 idxStart, u32 idxEnd) {
  return s.subarray(idxStart, idxEnd);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 */
template <typename T>
inline Slice<T> subarray(Slice<T> s, Range<u32> range) {
  return s.subarray(range);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 */
template <typename T>
inline Slice<T> subarray(Slice<T> s, Span<u32> span) {
  return s.subarray(span);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 */
template <typename T>
inline Slice<T> subarray(Slice<T> s, u32 idxStart) {
  return s.subarray(idxStart);
}

/**
 * \deprecated Prefer Slice<T>::shrinkFromLeft
 */
template <typename T>
inline void shrinkFromLeft(Slice<T> *target) {
  target->shrinkFromLeft();
}

/**
 * \deprecated Prefer Slice<T>::empty
 */
template <typename T>
inline b32 empty(Slice<T> s) {
  return s.empty() ? b32(1) : b32(0);
}

/**
 * \brief A macro that can be used to supply a slice (usually a Slice<char>) as
 * an argument to a printf-style function when using a directive like "%.*s".
 */
#define FMT_SLICE(s) (s).length, (s).data

/**
 * Casts a slice from one type to another.
 */
template <typename D, typename S>
Slice<D> cast(Slice<S> in) {
  return in.cast<D>();
}

template <typename T>
inline void copyElementsInto(Slice<T> s,
                             const T *src,
                             u32 numElements,
                             u32 offset = 0) {
  Slice<const T> src = {src, numElements};
  s.subarray(offset).copy(src);
}

/**
 * \deprecated Prefer Slice<T>::byteLength
 */
template <typename T>
inline u64 byteLength(Slice<T> s) {
  return s.byteLength();
}

/**
 * Creates a mutable slice from a C array.
 */
template <typename T, size_t N>
Slice<T> sliceFrom(T (&p)[N]) {
  return {p, N};
}

/**
 * Creates a slice from a C array.
 */
template <typename T, size_t N>
constexpr Slice<const T> sliceFrom(const T (&p)[N]) {
  return {p, N};
}

/**
 * \deprecated Prefer Slice<T>::contains
 */
template <typename T>
bool contains(Slice<T> s, const T &needle) {
  return s.contains(needle);
}

/**
 * \deprecated Prefer Slice<T>::contains
 */
template <typename T>
bool contains(Slice<const T> s, const T &needle) {
  return s.contains(needle);
}

/**
 * \deprecated Prefer Slice<T>::all
 */
template <typename T, typename F>
bool all(Slice<T> list, F &&condition) {
  return list.all(condition);
}

/**
 * \deprecated Prefer Slice<T>::any
 */
template <typename T, typename F>
bool any(Slice<T> list, F &&condition, u32 &index) {
  return list.any(condition, index);
}

/**
 * \deprecated Prefer Slice<T>::any
 */
template <typename T, typename F>
bool any(Slice<T> list, F &&condition) {
  return list.any(condition);
}

/**
 * \deprecated Prefer Slice<T>::reverse
 */
template <typename T>
void reverse(Slice<T> s) {
  s.reverse();
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
