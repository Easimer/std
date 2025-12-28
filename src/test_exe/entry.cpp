/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/Arena.h>
#include <std/Check.h>
#include <std/log/log.h>
#include <std/Slice.hpp>
#include <std/Testing.hpp>

#include <stdlib.h>

static const u32 SIZ_ARENA = 32 * 1024;

static Arena makeArena() {
  u8 *base = reinterpret_cast<u8 *>(malloc(SIZ_ARENA));
  u8 *end = base + SIZ_ARENA;
  return {base, end};
}

SN_TEST_MAIN;

int main(int numArgs, char **arrArgs) {
  Arena arena0 = makeArena();
  Arena arena1 = makeArena();

  SnTestStats stats;
  testMain(&arena0, &arena1, &stats);

  u32 numFail = stats.numTotal - stats.numSuccess;
  f32 percentSuccess = stats.numSuccess / f32(stats.numTotal) * 100.0f;
  f32 percentFail = numFail / f32(stats.numTotal) * 100.0f;

  printf("Successful tests: %u/%u (%.2f%%)\n", stats.numSuccess, stats.numTotal,
         percentSuccess);
  printf("Failed tests: %u/%u (%.2f%%)\n", numFail, stats.numTotal,
         percentFail);

  return 0;
}
