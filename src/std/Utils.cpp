/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/SliceUtils.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

b32 compareAsString(Slice<const char> left, Slice<const char> right) {
  if (left.data == nullptr || right.data == nullptr) {
    return left.data == right.data;
  }

  if (left.length != right.length) {
    return false;
  }

  return memcmp(left.data, right.data, left.length) == 0;
}

Slice<const char> fromCStr(const char *s) {
  size_t len = strlen(s);
  return {s, u32(len)};
}

Slice<const char> fromCStrWithZero(const char* s) {
  size_t len = strlen(s);
  return {s, u32(len) + 1};
}

extern "C" i32 countLeadingZeros(u32 x) {
#if defined(_MSC_VER)
  static_assert(sizeof(unsigned long) == 4);
  unsigned long index;
  if (_BitScanReverse(&index, x)) {
    // `index` contains the position of the first set bit, which is counted from
    // the LSB. Compute the number of leading zeros:
    return 32 - index - 1;
  } else {
    return 32;
  }
#else
  if (x == 0) {
    return 32;
  }

  return (i32)__builtin_clz(x);
#endif
}

extern "C" i32 countLeadingZeros64(u64 x) {
#if defined(_MSC_VER)
  static_assert(sizeof(unsigned __int64) == 8);
  unsigned long index;
  if (_BitScanReverse64(&index, x)) {
    // `index` contains the position of the first set bit, which is counted from
    // the LSB. Compute the number of leading zeros:
    return 64 - index - 1;
  } else {
    return 64;
  }
#else
  if (x == 0) {
    return 64;
  }

  return (i32)__builtin_clzll(x);
#endif
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
