/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Check.h"
#include "std/Optional.hpp"
#include "std/Types.h"

#include <string.h>

/** @file Slice.hpp */

/**
 * \defgroup Slice Slice
 * @{
 */

/**
 * \brief A span with a start index and the number of elements.
 */
template <typename T>
struct Span {
  /** \brief Index of the first element. */
  T start;
  /** \brief The number of elements */
  T count;

  bool empty() const { return count == 0; }

  bool contains(Span<T> inner) const {
    if (empty() || inner.empty() || count < inner.count) {
      return false;
    }

    Span<u32> diff = {start, count - inner.count};
    return diff.start <= inner.start &&
           inner.start <= (diff.start + diff.count);
  }
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
    const T &value;
    size_t index;
  };

  const T *const data;
  size_t index;

  bool operator!=(const SliceLegacyIterator<T> &other) const {
    return index != other.index;
  }

  void operator++() { index++; }

  Element operator*() { return {data[index], index}; }
};

template <typename T>
using SliceIterator = SliceLegacyIterator<T>;

/**
 * \brief A view on an immutable, contiguous array of T values. The values are
 * not owned by the slice.
 *
 * This can be iterated with a for-each expression; the iterator yields
 * `[value: const T&, index: size_t]` pairs.
 */
template <typename T>
struct Slice {
  using KeyT = size_t;
  using ValueT = const T;

  const T *data = nullptr;
  size_t length = 0;

  /** \brief Constructs an empty slice. */
  constexpr Slice() : data(nullptr), length(0) {}
  /** \brief Constructs a slice over an array. */
  constexpr Slice(const T *data, size_t length) : data(data), length(length) {}
  /** \brief Constructs a slice over a single value. */
  constexpr Slice(const T &value) : data(&value), length(1) {}

  const T &operator[](size_t i) const {
    DCHECK(data != nullptr);
    DCHECK(i < length);
    return data[i];
  }

  SliceLegacyIterator<T> begin() const { return {data, 0}; }
  SliceLegacyIterator<T> end() const { return {data, length}; }

  /**
   * \brief Returns an immutable view on the same elements as this slice.
   * \deprecated Slice<T> is always read-only now
   */
  [[deprecated]] Slice<T> asConst() const { return {data, length}; }

  /**
   * \brief Tests equality with a slice.
   *
   * Elements are compared with the equality operator.
   */
  bool operator==(Slice<T> other) const {
    if (length != other.length) {
      return false;
    }

    for (auto [elem, i] : other) {
      if (data[i] != elem) {
        return false;
      }
    }

    return true;
  }

  /**
   * \brief Tests inequality with a slice.
   *
   * Elements are compared with the inequality operator.
   */
  bool operator!=(Slice<T> other) const {
    if (length != other.length) {
      return true;
    }

    for (auto [elem, i] : other) {
      if (data[i] != elem) {
        return true;
      }
    }

    return false;
  }

  /**
   * \brief Tests whether this slice is empty.
   */
  bool empty() const { return length == 0; }

  /**
   * \brief Returns the size of this slice in **bytes**.
   */
  size_t byteLength() const { return length * sizeof(T); }

  /**
   * \brief Casts a slice to another type.
   */
  template <typename D>
  Slice<D> cast() const {
    static_assert(sizeof(D) < sizeof(T) || (sizeof(D) % sizeof(T)) == 0);
    static_assert(sizeof(T) < sizeof(D) || (sizeof(T) % sizeof(D)) == 0);
    Slice<D> ret;
    ret.data = (const D *)data;
    ret.length = length * sizeof(T) / sizeof(D);
    return ret;
  }

  /**
   * \brief Tries to find the first element in the slice that is equal to
   * `needle` and returns its index.
   */
  Optional<size_t> indexOf(const T &needle) const {
    for (size_t i = 0; i < length; i++) {
      if ((*this)[i] == needle) {
        return i;
      }
    }

    return {};
  }

  /**
   * \deprecated
   */
  bool indexOf(const T &needle, u32 *out) const {
    Optional<size_t> res = indexOf(needle);
    if (!res.hasValue()) {
      return false;
    }

    size_t idx = res.value();
    if (idx > 0xFFFFFFFF) {
      return false;
    }

    *out = u32(idx);
    return true;
  }

  /**
   * \brief Tries to find the last element in the slice that is equal to
   * `needle` and returns its index.
   */
  Optional<size_t> lastIndexOf(const T &needle) const {
    // NOTE(danielm): condition becomes false after `i` underflows
    for (size_t i = length - 1; i < length; i--) {
      if ((*this)[i] == needle) {
        return i;
      }
    }

    return {};
  }

  /**
   * \deprecated
   */
  bool lastIndexOf(const T &needle, u32 *out) const {
    Optional<size_t> res = lastIndexOf(needle);
    if (!res.hasValue()) {
      return false;
    }

    size_t idx = res.value();
    if (idx > 0xFFFFFFFF) {
      return false;
    }

    *out = u32(idx);
    return true;
  }

  /**
   * \brief Tests whether the slice contains an element that is equal to
   * `needle`.
   */
  bool contains(const T &needle) const {
    u32 discard;
    return indexOf(needle, &discard);
  }

  /**
   * \brief Returns a new slice on the same data. The starting index is
   * inclusive and the end index is exclusive.
   *
   * The specified range is clamped:
   * - `idxEnd` **can** be less than `idxStart`, in which case an empty slice is
   * returned.
   * - The specified range can be partially or completely out of range and this
   * function will never return a slice that is outside of the bounds of `this`.
   *
   * \param idxStart Element to begin at; inclusive.
   * \param idxEnd Element to end at; exclusive.
   */
  Slice<T> subarray(size_t idxStart, size_t idxEnd) const {
    if (idxEnd <= idxStart || length <= idxStart) {
      return {nullptr, 0};
    }

    if (length < idxEnd) {
      idxEnd = length;
    }

    size_t len = idxEnd - idxStart;
    const T *start = this->data + idxStart;

    return {start, len};
  }

  Slice<T> subarray(Range<u32> range) const {
    return subarray(range.start, range.end);
  }

  Slice<T> subarray(Span<u32> span) const { return subarray(rangeFrom(span)); }

  Slice<T> subarray(Range<size_t> range) const {
    return subarray(range.start, range.end);
  }

  Slice<T> subarray(Span<size_t> span) const {
    return subarray(rangeFrom(span));
  }

  /**
   * \brief Returns a new slice that's looking at the same data, starting at
   * `idxStart` up until the end of `this`.
   *
   * The specified range is clamped:
   * - `idxStart` can be out of range, in which case an empty slice is returned.
   *
   * \param idxStart Element to begin at; inclusive.
   */
  Slice<T> subarray(size_t idxStart) const {
    return subarray(idxStart, length);
  }
  Slice<T> subarray(u32 idxStart) const { return subarray(idxStart, length); }

  /**
   * \brief Steps the slice forward by at most N elements and decreases its
   * length accordingly. This "removes" the first N elements from the view.
   * \returns The new length of the slice
   */
  size_t shift(size_t numElements = 1) {
    if (numElements < length) {
      DCHECK(data != nullptr);
      data += numElements;
      length -= numElements;
    } else {
      data = nullptr;
      length = 0;
    }

    return length;
  }

  size_t pop(size_t numElements = 1) {
    if (numElements < length) {
      length -= numElements;
    } else {
      data = nullptr;
      length = 0;
    }

    return length;
  }

  /**
   * \brief Steps the slice forward by N elements and decreases its length
   * accordingly. The slice must have at least `numElements` elements.
   */
  Slice<T> &shrinkFromLeftByCount(size_t numElements) {
    CHECK(data != nullptr);
    CHECK(length >= numElements);
    data += numElements;
    length -= numElements;
    return *this;
  }

  /**
   * \brief Steps the slice forward by one element and decreases its length.
   * The slice must not be empty.
   */
  Slice<T> &shrinkFromLeft() { return shrinkFromLeftByCount(1); }

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
   */
  template <typename F>
  Optional<size_t> any(F &&condition) const {
    for (auto [elem, idxElem] : (*this)) {
      if (condition(elem)) {
        return idxElem;
      }
    }

    return {};
  }

  template <typename F>
  [[deprecated]] bool any(F &&condition, u32 &index) const {
    Optional<size_t> res = any(condition);
    if (!res.hasValue()) {
      return false;
    }

    size_t idx = res.value();
    if (idx > 0xFFFFFFFF) {
      return false;
    }

    index = idx;
    return true;
  }

  /**
   * \brief Tests that this slice starts with the specified prefix.
   */
  bool startsWith(Slice<T> prefix) const {
    if (prefix.empty()) {
      return true;
    }

    if (length < prefix.length) {
      return false;
    }

    return subarray(0, prefix.length) == prefix;
  }

  /**
   * \brief Tests that this slice ends with the specified suffix.
   */
  bool endsWith(Slice<T> suffix) const {
    if (suffix.empty()) {
      return true;
    }

    if (length < suffix.length) {
      return false;
    }

    return subarray(length - suffix.length) == suffix;
  }

  size_t count(const T &value) const {
    size_t ret = 0;
    for (auto [elem, _] : *this) {
      if (elem == value) {
        ret += 1;
      }
    }
    return ret;
  }

  template <typename F>
  size_t countIf(F &&condition) const {
    size_t ret = 0;
    for (auto [elem, _] : *this) {
      if (condition(elem)) {
        ret += 1;
      }
    }
    return ret;
  }
};

template <typename T>
struct MutSliceLegacyIterator {
  using Value = T;

  struct Element {
    T &value;
    size_t index;
  };

  T *const data;
  size_t index;

  bool operator!=(const MutSliceLegacyIterator<T> &other) const {
    return index != other.index;
  }

  void operator++() { index++; }

  Element operator*() { return {data[index], index}; }
};

/**
 * \brief A view on a mutable, contiguous array of T values. The values are not
 * owned by the slice.
 *
 * This can be iterated with a for-each expression; the iterator yields
 * `[value: T&, index: size_t]` pairs.
 */
template <typename T>
struct MutSlice {
  using KeyT = size_t;
  using ValueT = T;

  T *data;
  size_t length;

  /** \brief Constructs an empty slice. */
  constexpr MutSlice() : data(nullptr), length(0) {}
  /** \brief Constructs a slice over an array. */
  constexpr MutSlice(T *data, size_t length) : data(data), length(length) {}
  /** \brief Constructs a slice over a single value. */
  constexpr MutSlice(T &value) : data(&value), length(1) {}

  T &operator[](size_t i) const {
    DCHECK(data != nullptr);
    DCHECK(i < length);
    return data[i];
  }

  MutSliceLegacyIterator<T> begin() const { return {data, 0}; }
  MutSliceLegacyIterator<T> end() const { return {data, length}; }

  Slice<T> asSlice() const { return {static_cast<const T *>(data), length}; }

  operator Slice<T>() const { return asSlice(); }

  /**
   * \brief Tests equality with a slice.
   *
   * Elements are compared with the equality operator.
   */
  bool operator==(Slice<T> other) const { return asSlice() == other; }

  /**
   * \brief Tests inequality with a slice.
   *
   * Elements are compared with the inequality operator.
   */
  bool operator!=(Slice<T> other) const { return asSlice() != other; }

  /**
   * \brief Tests whether this slice is empty.
   */
  bool empty() const { return asSlice().empty(); }

  /**
   * \brief Returns the size of this slice in **bytes**.
   */
  size_t byteLength() const { return asSlice().byteLength(); }

  /**
   * \brief Casts a slice to another type.
   */
  template <typename D>
  MutSlice<D> cast() const {
    static_assert(sizeof(D) < sizeof(T) || (sizeof(D) % sizeof(T)) == 0);
    static_assert(sizeof(T) < sizeof(D) || (sizeof(T) % sizeof(D)) == 0);
    return MutSlice<D>(static_cast<const D *>(data),
                       length * sizeof(T) / sizeof(D));
  }

  /**
   * \brief Tries to find the first element in the slice that is equal to
   * `needle` and returns its index.
   */
  Optional<size_t> indexOf(const T &needle) const {
    return asSlice().indexOf(needle);
  }

  /**
   * \brief Tries to find the last element in the slice that is equal to
   * `needle` and returns its index.
   */
  Optional<size_t> lastIndexOf(const T &needle) const {
    return asSlice().lastIndexOf(needle);
  }

  /**
   * \brief Tests whether the slice contains an element that is equal to
   * `needle`.
   */
  bool contains(const T &needle) const { return asSlice().contains(needle); }

  /**
   * \brief Returns a new slice on the same data. The starting index is
   * inclusive and the end index is exclusive.
   *
   * The specified range is clamped:
   * - `idxEnd` **can** be less than `idxStart`, in which case an empty slice is
   * returned.
   * - The specified range can be partially or completely out of range and this
   * function will never return a slice that is outside of the bounds of `this`.
   *
   * \param idxStart Element to begin at; inclusive.
   * \param idxEnd Element to end at; exclusive.
   */
  MutSlice<T> subarray(size_t idxStart, size_t idxEnd) const {
    if (idxEnd <= idxStart || length <= idxStart) {
      return {nullptr, 0};
    }

    if (length < idxEnd) {
      idxEnd = length;
    }

    size_t len = idxEnd - idxStart;
    T *start = this->data + idxStart;

    return {start, len};
  }

  MutSlice<T> subarray(Range<size_t> range) const {
    return subarray(range.start, range.end);
  }

  MutSlice<T> subarray(Span<size_t> span) const {
    return subarray(rangeFrom(span));
  }

  /**
   * \brief Returns a new slice that's looking at the same data, starting at
   * `idxStart` up until the end of `this`.
   *
   * The specified range is clamped:
   * - `idxStart` can be out of range, in which case an empty slice is returned.
   *
   * \param idxStart Element to begin at; inclusive.
   */
  MutSlice<T> subarray(size_t idxStart) const {
    return subarray(idxStart, length);
  }

  /**
   * \brief Steps the slice forward by N elements and decreases its length
   * accordingly. The slice must have at least `numElements` elements.
   */
  MutSlice<T> &shrinkFromLeftByCount(size_t numElements) {
    CHECK(data != nullptr);
    CHECK(length >= numElements);
    data += numElements;
    length -= numElements;
    return *this;
  }

  MutSlice<T> &shrinkFromLeft() { return shrinkFromLeftByCount(1); }

  /**
   * \brief Copies all elements from `source` into the beginning of this slice.
   */
  MutSlice<T> &copy(Slice<T> source) {
    if (source.empty()) {
      return *this;
    }

    CHECK(source.length <= length);
    for (auto [elem, i] : source) {
      (*this)[i] = elem;
    }

    return *this;
  }

  /**
   * \brief Reverses the contents of this slice.
   */
  MutSlice<T> &reverse() {
    if (empty()) {
      return *this;
    }

    MutSlice<T> &s = *this;
    const size_t idxLast = s.length - 1;
    const size_t idxMid = s.length / 2;
    for (size_t idxCur = 0; idxCur < idxMid; idxCur++) {
      size_t idxMirror = idxLast - idxCur;
      T t = s[idxCur];
      s[idxCur] = s[idxMirror];
      s[idxMirror] = t;
    }

    return *this;
  }

  /**
   * \brief Copies all elements from `source` into the beginning of this slice
   * using memcpy.
   */
  MutSlice<T> &memcopy(Slice<T> source) {
    if (source.empty()) {
      return *this;
    }

    CHECK(source.length <= length);
    ::memcpy(data, source.data, source.length * sizeof(T));

    return *this;
  }

  /**
   * \brief Replaces all instances of a `prev` with `next`
   */
  MutSlice<T> &replace(const T &prev, const T &next) {
    for (auto [val, _] : *this) {
      if (val == prev) {
        val = next;
      }
    }

    return *this;
  }

  /**
   * \brief Fills the slice with the specified value.
   */
  MutSlice<T> &fill(const T &value) {
    for (size_t i = 0; i < length; i++) {
      (*this)[i] = value;
    }

    return (*this);
  }

  /**
   * \brief Copies the contents of this slice to the destination. Each element
   * of type `S` will be converted to type `D`.
   */
  template <typename S>
  MutSlice<T> &copyWithConversionFrom(Slice<S> src) {
    DCHECK(src.length == length);

    for (size_t i = 0; i < src.length; i++) {
      const S &elem = src[i];
      (*this)[i] = T(elem);
    }

    return (*this);
  }

  /**
   * \brief Steps the slice forward by at most N elements and decreases its
   * length accordingly. This "removes" the first N elements from the view.
   * \returns The new length of the slice
   */
  size_t shift(size_t numElements = 1) {
    if (numElements < length) {
      DCHECK(data != nullptr);
      data += numElements;
      length -= numElements;
    } else {
      data = nullptr;
      length = 0;
    }

    return length;
  }

  size_t pop(size_t numElements = 1) {
    if (numElements < length) {
      length -= numElements;
    } else {
      data = nullptr;
      length = 0;
    }

    return length;
  }

  /**
   * \brief Tests whether every element of this slice satisfies `condition`.
   * \param condition A callable object that takes in `T&` and returns a
   * boolean.
   */
  template <typename F>
  bool all(F &&condition) const {
    return asSlice().all(condition);
  }

  /**
   * \brief Tests whether any element of this slice satisfies `condition`.
   * \param condition A callable object that takes in `T&` and returns a
   * boolean.
   */
  template <typename F>
  Optional<size_t> any(F &&condition) const {
    return asSlice().any(condition);
  }

  /**
   * \brief Tests that this slice starts with the specified prefix.
   */
  bool startsWith(Slice<T> prefix) const {
    return asSlice().startsWith(prefix);
  }

  /**
   * \brief Tests that this slice ends with the specified suffix.
   */
  bool endsWith(Slice<T> suffix) const { return asSlice().endsWith(suffix); }

  size_t count(const T &value) const { return asSlice().count(value); }

  template <typename F>
  size_t countIf(F &&condition) const {
    return asSlice().countIf(condition);
  }
};

#define SLICE_DEFINE_COPY_SPECIALIZATION(T)                \
  template <>                                              \
  inline MutSlice<T> &MutSlice<T>::copy(Slice<T> source) { \
    return memcopy(source);                                \
  }

SLICE_DEFINE_COPY_SPECIALIZATION(u8);
SLICE_DEFINE_COPY_SPECIALIZATION(u16);
SLICE_DEFINE_COPY_SPECIALIZATION(u32);
SLICE_DEFINE_COPY_SPECIALIZATION(u64);

SLICE_DEFINE_COPY_SPECIALIZATION(i8);
SLICE_DEFINE_COPY_SPECIALIZATION(i16);
SLICE_DEFINE_COPY_SPECIALIZATION(i32);
SLICE_DEFINE_COPY_SPECIALIZATION(i64);

SLICE_DEFINE_COPY_SPECIALIZATION(f32);
SLICE_DEFINE_COPY_SPECIALIZATION(f64);

/**
 * \deprecated Prefer Slice<T>::shrinkFromLeftByCount
 * \private
 */
template <typename T>
[[deprecated]] inline void shrinkFromLeftByCount(Slice<T> *target,
                                                 u32 numElements) {
  target->shrinkFromLeftByCount(numElements);
}

/**
 * \deprecated Prefer Slice<T>::indexOf
 * \private
 */
template <typename T>
[[deprecated]] inline b32 indexOf(Slice<const T> s, const T &needle, u32 *out) {
  return s.indexOf(needle, out) ? b32(1) : b32(0);
}

/**
 * \deprecated Prefer Slice<T>::indexOf
 * \private
 */
template <typename T>
[[deprecated]] inline b32 indexOf(Slice<T> s, const T &needle, u32 *out) {
  return indexOf(s.asConst(), needle, out);
}

/**
 * \deprecated Prefer Slice<T>::lastIndexOf
 * \private
 */
template <typename T>
[[deprecated]] inline b32 lastIndexOf(Slice<T> s, const T &needle, u32 *out) {
  return s.lastIndexOf(needle, out);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 * \private
 */
template <typename T>
[[deprecated]] inline Slice<T> subarray(Slice<T> s, u32 idxStart, u32 idxEnd) {
  return s.subarray(idxStart, idxEnd);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 * \private
 */
template <typename T>
[[deprecated]] inline Slice<T> subarray(Slice<T> s, Range<u32> range) {
  return s.subarray(range);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 * \private
 */
template <typename T>
[[deprecated]] inline Slice<T> subarray(Slice<T> s, Span<u32> span) {
  return s.subarray(span);
}

/**
 * \deprecated Prefer Slice<T>::subarray
 * \private
 */
template <typename T>
[[deprecated]] inline Slice<T> subarray(Slice<T> s, u32 idxStart) {
  return s.subarray(idxStart);
}

/**
 * \deprecated Prefer Slice<T>::shrinkFromLeft
 * \private
 */
template <typename T>
[[deprecated]] inline void shrinkFromLeft(Slice<T> *target) {
  target->shrinkFromLeft();
}

/**
 * \deprecated Prefer Slice<T>::empty
 * \private
 */
template <typename T>
[[deprecated]] inline b32 empty(Slice<T> s) {
  return s.empty() ? b32(1) : b32(0);
}

/**
 * \brief A macro that can be used to supply a slice (usually a Slice<char>) as
 * an argument to a printf-style function when using a conversion specification
 * like "%.*s". WARNING: the conversion precision specifier (`".*"`) takes in
 * an argument of type `int` while a Slice stores its length in `size_t`,
 * therefore the behavior of printing slices longer than `INT_MAX` is undefined!
 */
#define FMT_SLICE(S) ((int)(S).length), ((S).data)

/**
 * \brief Casts a slice from one type to another.
 */
template <typename D, typename S>
[[deprecated]] Slice<D> cast(Slice<S> in) {
  return in.template cast<D>();
}

template <typename T>
inline void copyElementsInto(MutSlice<T> s,
                             const T *src,
                             u32 numElements,
                             u32 offset = 0) {
  Slice<const T> srcs = {src, numElements};
  s.subarray(offset).copy(srcs);
}

/**
 * \deprecated Prefer Slice<T>::byteLength
 * \private
 */
template <typename T>
[[deprecated]] inline u64 byteLength(Slice<T> s) {
  return s.byteLength();
}

/**
 * Creates a slice from a C array.
 */
template <typename T, size_t N>
MutSlice<T> sliceFrom(T (&p)[N]) {
  return {p, N};
}

/**
 * Creates a slice from a C array.
 */
template <typename T, size_t N>
constexpr Slice<T> sliceFrom(const T (&p)[N]) {
  return {p, N};
}

/**
 * \deprecated Prefer Slice<T>::contains
 * \private
 */
template <typename T>
[[deprecated]] bool contains(Slice<T> s, const T &needle) {
  return s.contains(needle);
}

/**
 * \deprecated Prefer Slice<T>::contains
 */
template <typename T>
[[deprecated]] bool contains(Slice<const T> s, const T &needle) {
  return s.contains(needle);
}

/**
 * \deprecated Prefer Slice<T>::all
 * \private
 */
template <typename T, typename F>
[[deprecated]] bool all(Slice<T> list, F &&condition) {
  return list.all(condition);
}

/**
 * \deprecated Prefer Slice<T>::any
 * \private
 */
template <typename T, typename F>
[[deprecated]] bool any(Slice<T> list, F &&condition, u32 &index) {
  return list.any(condition, index);
}

/**
 * \deprecated Prefer Slice<T>::any
 * \private
 */
template <typename T, typename F>
[[deprecated]] bool any(Slice<T> list, F &&condition) {
  return list.any(condition);
}

/**
 * \deprecated Prefer Slice<T>::reverse
 * \private
 */
template <typename T>
[[deprecated]] void reverse(MutSlice<T> s) {
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

/**@}*/
