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

