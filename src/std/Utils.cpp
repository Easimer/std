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
