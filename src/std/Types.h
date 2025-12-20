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
i32 countLeadingZeros(u32 x);
/**
 * \brief Count leading zeros.
 * \returns The number of leading zeros. When x is zero, the result is 64,
 * similarly to Rust's `u64::leading_zeros`.
 */
i32 countLeadingZeros64(u64 x);

#if __cplusplus
}
#endif
