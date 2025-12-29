#include <std/Arena.h>
#include <std/Check.h>
#include <std/log/log.h>
#include <std/Slice.hpp>
#include <std/Testing.hpp>

#include <stdlib.h>

SN_TEST_MUST_FAIL(Check, assertsOnFalseCondition) {
  CHECK(false);
}

SN_TEST(Check, doesNotAssertOnTrueCondition) {
  CHECK(true);
}

