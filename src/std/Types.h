/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef uint32_t b32;

#if __cplusplus
extern "C" {
#endif

/**
 * \brief Count leading zeros.
 * \returns The number of leading zeros. When x is zero, the result is 32,
 * similarly to Rust's `u32::leading_zeros`.
 */
static inline i32 countLeadingZeros(u32 x) {
#if defined(_MSC_VER)
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

/**
 * \brief Count leading zeros.
 * \returns The number of leading zeros. When x is zero, the result is 64,
 * similarly to Rust's `u64::leading_zeros`.
 */
static inline i32 countLeadingZeros64(u64 x) {
#if defined(_MSC_VER)
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

u8 mipmapGetLevelCount(u32 width, u32 height);
u8 mipmapGetLevelCount3(u32 width, u32 height, u32 depth);
u8 mipmapGetLevelDim(u32 dim0, u8 level);

#if __cplusplus
}
#endif

#define ARG_UNUSED(x) (void)(x)
