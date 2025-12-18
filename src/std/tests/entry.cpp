#include <std/Arena.h>
#include <std/Check.h>
#include <std/log/log.h>
#include <std/Slice.hpp>
#include <std/Testing.hpp>

#include <stdlib.h>

static const u32 SIZ_ARENA = 32 * 1024;

SN_TEST_MUST_FAIL(Check, assertsOnFalseCondition) {
  CHECK(false);
}

SN_TEST(Check, doesNotAssertOnTrueCondition) {
  CHECK(true);
}

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
