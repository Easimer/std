#include <std/Check.h>
#include <std/Types.h>
#include <std/Testing.hpp>

SN_TEST(countLeadingZeros, zero) {
  CHECK(countLeadingZeros(0) == 32);
}

SN_TEST(countLeadingZeros, testAll) {
  for (u32 i = 0; i < 32; i++) {
    u32 x = 1 << i;
    i32 numLeadingZeros = countLeadingZeros(x);
    CHECK(numLeadingZeros == 32 - i - 1);
  }
}

SN_TEST(countLeadingZeros64, zero) {
  CHECK(countLeadingZeros64(0) == 64);
}

SN_TEST(countLeadingZeros64, testAll) {
  for (u32 i = 0; i < 64; i++) {
    u64 x = u64(1) << i;
    i32 numLeadingZeros = countLeadingZeros64(x);
    CHECK(numLeadingZeros == 64 - i - 1);
  }
}
