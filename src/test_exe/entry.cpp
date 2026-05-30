/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/Arena.h>
#include <std/Check.h>
#include <std/Chronometry.h>
#include <std/log.h>
#include <std/Slice.hpp>
#include <std/Testing.hpp>

#include <stdio.h>
#include <stdlib.h>

static const u32 SIZ_ARENA = 256 * 1024;

static Arena makeArena() {
  u8 *base = reinterpret_cast<u8 *>(malloc(SIZ_ARENA));
  u8 *end = base + SIZ_ARENA;
  return {base, end};
}

SN_TEST_MAIN;

static void printRes(const SnTestResult *results) {
  if (results->ok) {
    printf("{  OK  } ");
  } else {
    printf("{ FAIL } ");
  }

  const SnTest *test = results->test;
  printf("[%s] %s [%.03f ms]\n", test->suiteName, test->name,
         results->duration * 1000.0);
}

int main(int numArgs, char **arrArgs) {
  Arena arena0 = makeArena();
  Arena arena1 = makeArena();
  Arena arenaResults = makeArena();

  SnTestStats stats;
  TimePoint t_start = chrono_getCurrentTime();
  SnTestResult* results = testMain(&arena0, &arena1, &arenaResults, &stats);
  TimePoint t_end = chrono_getCurrentTime();

  u32 numFail = stats.numTotal - stats.numSuccess;
  f32 percentSuccess = stats.numSuccess / f32(stats.numTotal) * 100.0f;
  f32 percentFail = numFail / f32(stats.numTotal) * 100.0f;

  printf("\n");

  if (numFail != 0) {
    // Print failed tests first
    SnTestResult *cur = results;
    while (cur != nullptr) {
      if (cur->ok) {
        cur = cur->next;
        continue;
      }

      printRes(cur);
      cur = cur->next;
    }
    printf("\n");
  }

  {
    SnTestResult *cur = results;
    while (cur != nullptr) {
      if (!cur->ok) {
        cur = cur->next;
        continue;
      }

      printRes(cur);
      cur = cur->next;
    }
    printf("\n");
  }

  printf("Successful tests: %u/%u (%.2f%%)\n", stats.numSuccess, stats.numTotal,
         percentSuccess);
  printf("Failed tests:     %u/%u (%.2f%%)\n", numFail, stats.numTotal,
         percentFail);

  free(arenaResults.beg);
  free(arena1.beg);
  free(arena0.beg);

  f64 elapsed = chrono_secondsBetween(t_start, t_end);
  printf("Duration:         %f milliseconds\n", elapsed * 1000.0);

  int rc = (numFail == 0) ? 0 : 1;
  return rc;
}
