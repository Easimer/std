#include <std/Check.h>
#include <std/SegmentArray.hpp>
#include <std/Testing.hpp>

SN_TEST(SegmentArray, defaultConstructedIsEmpty) {
  Arena::Scope temp = getScratch(nullptr, 0);

  SegmentArray<u32> s(temp);

  CHECK(s.length == 0);
}

SN_TEST(SegmentArray, getSegmentForItem) {
  CHECK(SegmentArray<u32>::getSegmentForItem(0) == 0);
  CHECK(SegmentArray<u32>::getSegmentForItem(1) == 0);
  CHECK(SegmentArray<u32>::getSegmentForItem(63) == 0);
  CHECK(SegmentArray<u32>::getSegmentForItem(64) == 1);
  CHECK(SegmentArray<u32>::getSegmentForItem(192) == 2);
}

SN_TEST(SegmentArray, pushOne) {
  Arena::Scope temp = getScratch(nullptr, 0);

  SegmentArray<u32> s(temp);

  s.push(32);
  CHECK(s.length == 1);
  CHECK(s[0] == 32);
}

SN_TEST(SegmentArray, push67) {
  Arena::Scope temp = getScratch(nullptr, 0);

  SegmentArray<u32> s(temp);

  for (u32 i = 0; i < 67; i++) {
    s.push(i);
  }

  CHECK(s.length == 67);

  for (u32 i = 0; i < 67; i++) {
    CHECK(s[i] == i);
  }
}

SN_TEST(SegmentArray, multiPush) {
  Arena::Scope temp = getScratch(nullptr, 0);

  SegmentArray<u32> s(temp);

  const u32 values[3] = {11, 23, 31};

  s.push(sliceFrom(values));

  CHECK(s.length == 3);

  for (u32 i = 0; i < 3; i++) {
    CHECK(s[i] == values[i]);
  }
}

SN_TEST(SegmentArray, multiPushWholeSegment) {
  // Multipush where the number of pushed elements is equal to the size of
  // segment 0
  Arena::Scope temp = getScratch(nullptr, 0);

  SegmentArray<u32> s(temp);

  u32 arrBuf[64];
  Slice<u32> buf = sliceFrom(arrBuf);
  for (u32 i = 0; i < 64; i++) {
    buf[i] = i;
  }

  s.push(buf.asConst());

  CHECK(s.length == 64);
  for (u32 i = 0; i < 64; i++) {
    CHECK(s[i] == buf[i]);
  }
}

SN_TEST(SegmentArray, multiPushWithGrow) {
  // Multipush where the number of pushed elements is large enough that they
  // have to be copied in two batches
  Arena::Scope temp = getScratch(nullptr, 0);

  SegmentArray<u32> s(temp);

  u32 arrBuf[67];
  Slice<u32> buf = sliceFrom(arrBuf);
  for (u32 i = 0; i < 67; i++) {
    buf[i] = i;
  }

  s.push(buf.asConst());

  CHECK(s.length == 67);
  for (u32 i = 0; i < 67; i++) {
    CHECK(s[i] == buf[i]);
  }
}

SN_TEST(SegmentArray, multiPushWithGrowNonEmpty) {
  // Multipush where the number of pushed elements is large enough that they
  // have to be copied in two batches. The array is not empty before the push
  Arena::Scope temp = getScratch(nullptr, 0);

  SegmentArray<u32> s(temp);
  s.push(0);

  u32 arrBuf[67];
  Slice<u32> buf = sliceFrom(arrBuf);
  for (u32 i = 0; i < 67; i++) {
    buf[i] = i;
  }

  s.push(buf.asConst());

  CHECK(s.length == 68);
  CHECK(s[0] == 0);
  for (u32 i = 1; i < 68; i++) {
    CHECK(s[i] == buf[i - 1]);
  }
}
