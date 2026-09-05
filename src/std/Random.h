/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <std/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

static const u32 RAND_LCG_U32_A = 0x915F77F5;
static const u32 RAND_LCG_U32_C = 0xE817FB2D;

static const u64 RAND_LCG_U64_A = 0xD1342543DE82EF95ull;
static const u64 RAND_LCG_U64_C = 9754186451795953191ull;

static inline u32 rand_lcg_u32(u32 *state, u32 a, u32 c) {
  u32 ret = a * (*state) + c;
  *state = ret;
  return ret;
}

static inline u64 rand_lcg_u64(u64 *state, u64 a, u64 c) {
  u64 ret = a * (*state) + c;
  *state = ret;
  return ret;
}
/**
 * \brief Creates a floating-point value in the range [0, 1[ from random bits
 * \param bits A 32-bit random bit string from which the least-significant 23
 * bits will be used as the mantissa
 * \returns A floating-point value between [0, 1[
 */
static inline f32 f32FromBits_0_1(u32 bits) {
  f32 ret;
  // Extract 23 random bits and use it as the mantissa
  bits &= (~0xff800000);
  // Set sign to 0 and exponent to 127, creating a bit string that can be
  // interpreted as a float `(-1)^0 * 2^0 * 1.m` (where m is the 23 random bits
  // above), i.e. a float from 1.0 and 2.0 (inclusive-exclusive).
  bits |= 0x3f800000;
  ret = *(f32 *)&bits;
  // Shift the range from [1, 2[ to [0, 1[
  ret = ret - 1.0f;

  return ret;
}

/**
 * \brief Creates a floating-point value in the range [-1, 1[ from random bits
 * \param bits A 32-bit random bit string from which the least-significant 23
 * bits will be used as the mantissa
 * \returns A floating-point value between [-1, 1[
 */
static inline f32 f32FromBits_1_1(u32 bits) {
  f32 ret;
  // Extract 23 random bits and use it as the mantissa
  bits &= (~0xff800000);
  // Set sign to 0 and exponent to 128, creating a bit string that can be
  // interpreted as a float `(-1)^0 * 2^1 * 1.m` (where m is the 23 random bits
  // above), i.e. a float from 2.0 to 4.0 (inclusive-exclusive).
  bits |= 0x40000000;
  ret = *(f32 *)&bits;
  // Shift the range from [2, 4[ to [-1, 1[
  ret = ret - 3.0f;

  return ret;
}

/**
 * \brief Generates a random floating-point value in the range [0, 1[
 */
static inline f32 rand_lcg_f32_0_1(u32 *state, u32 a, u32 c) {
  u32 bits = rand_lcg_u32(state, a, c);
  return f32FromBits_0_1(bits);
}

/**
 * \brief Generates a random floating-point value in the range [-1, 1[
 */
static inline f32 rand_lcg_f32_1_1(u32 *state, u32 a, u32 c) {
  u32 bits = rand_lcg_u32(state, a, c);
  return f32FromBits_1_1(bits);
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
template <u32 A = RAND_LCG_U32_A, u32 C = RAND_LCG_U32_C>
struct RandLcgU32 {
  u32 state;

  RandLcgU32() : state(1140671485) {}
  RandLcgU32(const u32 &initialState) : state(initialState) {}

  u32 next() { return rand_lcg_u32(&state, A, C); }
};

template <u64 A = RAND_LCG_U64_A, u64 C = RAND_LCG_U64_C>
struct RandLcgU64 {
  u64 state;

  RandLcgU64() : state(2113148651ULL) {}
  RandLcgU64(const u64 &initialState) : state(initialState) {}

  u64 next() { return rand_lcg_u64(&state, A, C); }
};
#endif
