/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/Random.h>
#include <std/Testing.hpp>

SN_TEST(Random, rand_lcg_u32_zero) {
  u32 state = 0;
  u32 x = rand_lcg_u32(&state, RAND_LCG_U32_A, RAND_LCG_U32_C);
  ASSERT_NOT_EQUAL(x, 0);
}

SN_TEST(Random, rand_lcg_u64_zero) {
  u64 state = 0;
  u64 x = rand_lcg_u64(&state, RAND_LCG_U64_A, RAND_LCG_U64_C);
  ASSERT_NOT_EQUAL(x, 0);
}

SN_TEST(Random, rand_lcg_f32_0_1_withinRange) {
  u32 state = 1140671485;

  f32 sum = 0;

  const u32 N = 1 << 16;
  for (u32 i = 0; i < N; i++) {
    f32 x = rand_lcg_f32_0_1(&state, RAND_LCG_U32_A, RAND_LCG_U32_C);
    ASSERT_BETWEEN(x, 0.0f, 1.0f);
    sum += x;
  }

  f32 mean = sum / f32(N);
  ASSERT_BETWEEN(mean, 0.499f, 0.501f);
}

SN_TEST(Random, rand_lcg_f32_1_1_withinRange) {
  u32 state = 1140671485;

  f32 sum = 0;

  const u32 N = 1 << 16;
  for (u32 i = 0; i < N; i++) {
    f32 x = rand_lcg_f32_1_1(&state, RAND_LCG_U32_A, RAND_LCG_U32_C);
    ASSERT_BETWEEN(x, -1.0f, 1.0f);
    sum += x;
  }

  f32 mean = sum / f32(N);
  ASSERT_BETWEEN(mean, -0.001f, +0.001f);
}
