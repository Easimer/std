/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/SliceUtils.hpp"

b32 compareAsString(Slice<char> left, Slice<char> right) {
  if (left.data == nullptr || right.data == nullptr) {
    return left.data == right.data;
  }

  if (left.length != right.length) {
    return false;
  }

  return memcmp(left.data, right.data, left.length) == 0;
}

Slice<char> fromCStr(const char *s) {
  size_t len = strlen(s);
  return {s, u32(len)};
}

Slice<char> fromCStrWithZero(const char *s) {
  size_t len = strlen(s);
  return {s, u32(len) + 1};
}

#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

extern "C" u8 mipmapGetLevelCount(u32 width, u32 height) {
  if (width == 0 || height == 0) {
    return 0;
  }

  u8 n0w = (u8)countLeadingZeros(width);
  u8 n0h = (u8)countLeadingZeros(height);

  return 32 - MIN(n0w, n0h);
}

extern "C" u8 mipmapGetLevelCount3(u32 width, u32 height, u32 depth) {
  if (width == 0 || height == 0) {
    return 0;
  }

  u8 n0w = (u8)countLeadingZeros(width);
  u8 n0h = (u8)countLeadingZeros(height);
  u8 n0d = (u8)countLeadingZeros(depth);

  return 32 - MIN(MIN(n0w, n0h), n0d);
}

extern "C" u8 mipmapGetLevelDim(u32 dim0, u8 level) {
  u32 res = dim0 >> level;
  return res > 0 ? res : 1;
}

template <typename S, typename T, typename D>
concept SliceLike = requires(S s,
                             const S &other,
                             T value,
                             T *ptr,
                             size_t index,
                             size_t length) {
  // Can construct an empty slice
  { S() } -> std::same_as<S>;
  // Can construct from pointer + length
  { S(ptr, length) } -> std::same_as<S>;
  // Can construct from a single value
  { S(value) } -> std::same_as<S>;
  // Indexable using size_t
  { s[index] } -> std::convertible_to<const T &>;
  // Eq-comparable
  { s == other } -> std::same_as<bool>;
  // Neq-comparable
  { s != other } -> std::same_as<bool>;
  // Castable to D
  { s.template cast<D>() } -> std::convertible_to<Slice<D>>;
  // Emptyness can be queried
  { s.empty() } -> std::same_as<bool>;
  // Byte-length can be queried
  { s.byteLength() } -> std::same_as<size_t>;
  // Implements indexOf
  { s.indexOf(value) } -> std::same_as<Optional<size_t>>;
  // Implements lastIndexOf
  { s.lastIndexOf(value) } -> std::same_as<Optional<size_t>>;
  // Can check that contains element
  { s.contains(value) } -> std::same_as<bool>;
  // Can create subarray with 2 args
  { s.subarray(index, index) } -> std::convertible_to<Slice<T>>;
  // Can create subarray with 1 arg
  { s.subarray(index) } -> std::convertible_to<Slice<T>>;
  // Can create subarray with Range
  { s.subarray(Range<size_t>(index, index)) } -> std::convertible_to<Slice<T>>;
  // Can create subarray with Span
  { s.subarray(Span<size_t>(index, length)) } -> std::convertible_to<Slice<T>>;
  // Shiftable
  { s.shift(size_t(4)) } -> std::same_as<size_t>;
  // Shiftable (default arg)
  { s.shift() } -> std::same_as<size_t>;
  // Popable
  { s.pop(size_t(3)) } -> std::same_as<size_t>;
  // Popable (default arg)
  { s.pop() } -> std::same_as<size_t>;
  // Shrinkable from left by count
  { s.shrinkFromLeftByCount(1) } -> std::same_as<S &>;
  // Shrinkable from left
  { s.shrinkFromLeft() } -> std::same_as<S &>;
  // Can query whether a predicate is true for all elements
  {
    s.all([](const T &elem) { return true; })
    } -> std::same_as<bool>;
  // Can query whether a predicate is true for any elements and if so, returns
  // an index
  {
    s.any([](const T &elem) { return true; })
    } -> std::same_as<Optional<size_t>>;
  // Can query whether the slice starts with another slice
  { s.startsWith(Slice<T>()) } -> std::same_as<bool>;
  // Can query whether the slice ends with another slice
  { s.endsWith(Slice<T>()) } -> std::same_as<bool>;
  // Can query how many times a value occurs in the slice
  { s.count(value) } -> std::same_as<size_t>;
  // Can query how many times a value satisfying a predicate occurs in the slice
  {
    s.countIf([](const T &v) { return true; })
    } -> std::same_as<size_t>;
};

template <typename S, typename T, typename D>
concept MutSliceLike = requires(S s,
                                const S &other,
                                T value,
                                T *ptr,
                                size_t index,
                                size_t length) {
  requires SliceLike<S, T, D>;
  requires std::is_convertible_v<S, Slice<T>>;
  { s[index] } -> std::convertible_to<T &>;
  { s.shrinkFromLeftByCount(length) } -> std::same_as<S &>;
  { s.shrinkFromLeft() } -> std::same_as<S &>;
  { s.copy(other) } -> std::same_as<S &>;
  { s.memcopy(other) } -> std::same_as<S &>;
  { s.replace(value, value) } -> std::same_as<S &>;
  { s.fill(value) } -> std::same_as<S &>;
  { s.copyWithConversionFrom(Slice<D>()) } -> std::same_as<S &>;
};

static_assert(SliceLike<Slice<i32>, i32, f32>, "Slice must be a SliceLike!");
static_assert(MutSliceLike<MutSlice<i32>, i32, f32>,
              "MutSlice must be a MutSliceLike");
