#include <std/Check.h>
#include <std/Types.h>
#include <std/Testing.hpp>

SN_TEST(countLeadingZeros, zero) {
  CHECK(countLeadingZeros(0) == 32);
}

SN_TEST(countLeadingZeros, testAll) {
  for (i32 i = 0; i < 32; i++) {
    i32 x = 1 << i;
    i32 numLeadingZeros = countLeadingZeros(x);
    CHECK(numLeadingZeros == 32 - i - 1);
  }
}

SN_TEST(countLeadingZeros64, zero) {
  CHECK(countLeadingZeros64(0) == 64);
}

SN_TEST(countLeadingZeros64, testAll) {
  for (i32 i = 0; i < 64; i++) {
    i64 x = i64(1) << i;
    i32 numLeadingZeros = countLeadingZeros64(x);
    CHECK(numLeadingZeros == 64 - i - 1);
  }
}

SN_TEST(Mipmap, getLevelCount) {
  CHECK(mipmapGetLevelCount(1, 1) == 1);
  CHECK(mipmapGetLevelCount(4, 4) == 3);
  CHECK(mipmapGetLevelCount(128, 128) == 8);
  CHECK(mipmapGetLevelCount(1024, 1024) == 11);

  CHECK(mipmapGetLevelCount(4, 8) == 4);
  CHECK(mipmapGetLevelCount(128, 1) == 8);
  CHECK(mipmapGetLevelCount(1, 128) == 8);

  CHECK(mipmapGetLevelCount(0, 0) == 0);
  CHECK(mipmapGetLevelCount(32, 0) == 0);
  CHECK(mipmapGetLevelCount(0, 32) == 0);
}

SN_TEST(Mipmap, getLevelCount3) {
  CHECK(mipmapGetLevelCount3(1, 1, 1) == 1);
  CHECK(mipmapGetLevelCount3(4, 4, 4) == 3);
  CHECK(mipmapGetLevelCount3(128, 128, 128) == 8);

  CHECK(mipmapGetLevelCount3(4, 8, 4) == 4);
  CHECK(mipmapGetLevelCount3(128, 1, 64) == 8);
  CHECK(mipmapGetLevelCount3(1, 128, 16) == 8);

  CHECK(mipmapGetLevelCount3(0, 0, 0) == 0);
  CHECK(mipmapGetLevelCount3(32, 0, 0) == 0);
  CHECK(mipmapGetLevelCount3(0, 32, 0) == 0);
  CHECK(mipmapGetLevelCount3(0, 0, 32) == 0);
}

SN_TEST(Mipmap, getLevelDim) {
  CHECK(mipmapGetLevelDim(128, 0) == 128);
  CHECK(mipmapGetLevelDim(128, 1) == 64);
  CHECK(mipmapGetLevelDim(128, 7) == 1);
  CHECK(mipmapGetLevelDim(128, 8) == 1);
}
