#include <std/Check.h>
#include <std/FixedRingBuffer.hpp>
#include <std/Testing.hpp>

SN_TEST(FixedRingBuffer, defaultConstructedIsEmpty) {
  FixedRingBuffer<u32, 64> b;

  CHECK(b.empty());
  CHECK(!b.full());
  CHECK(b.size() == 0);
}

SN_TEST(FixedRingBuffer, pushIncreasesSize) {
  FixedRingBuffer<u32, 64> b;

  b.push(0);

  CHECK(!b.empty());
  CHECK(!b.full());
  CHECK(b.size() == 1);
}

SN_TEST(FixedRingBuffer, pushPop) {
  FixedRingBuffer<u32, 64> b;

  b.push(3);
  u32 x = b.pop();
  CHECK(x == 3);

  CHECK(b.empty());
  CHECK(!b.full());
  CHECK(b.size() == 0);
}

SN_TEST(FixedRingBuffer, full) {
  FixedRingBuffer<u32, 1> b;

  b.push(5);
  CHECK(b.full());
}

SN_TEST(FixedRingBuffer, order) {
  FixedRingBuffer<u32, 3> b;

  b.push(1);
  b.push(2);
  b.push(3);

  CHECK(b.pop() == 1);
  CHECK(b.pop() == 2);
  CHECK(b.pop() == 3);
}

SN_TEST(FixedRingBuffer, tryPushFailsOnFull) {
  FixedRingBuffer<u32, 1> b;

  b.push(5);
  bool res = b.tryPush(6);
  CHECK(!res);
}

SN_TEST(FixedRingBuffer, indexOverflow) {
  FixedRingBuffer<u32, 1> b;
  b.read = 0xFFFFFFFF;
  b.write = 0xFFFFFFFF;
  CHECK(b.empty());
  CHECK(!b.full());
  CHECK(b.size() == 0);

  b.push(5);

  CHECK(!b.empty());
  CHECK(b.full());
  CHECK(b.size() == 1);

  u32 x = b.pop();
  CHECK(x == 5);
}

SN_TEST(FixedRingBuffer, indexOverflow2) {
  FixedRingBuffer<u32, 2> b;
  b.read = 0xFFFFFFFF;
  b.write = 0xFFFFFFFF;
  CHECK(b.empty());
  CHECK(!b.full());
  CHECK(b.size() == 0);

  b.push(5);
  CHECK(b.size() == 1);
  b.push(6);
  CHECK(b.size() == 2);

  CHECK(b.pop() == 5);
  CHECK(b.size() == 1);

  CHECK(b.pop() == 6);
  CHECK(b.size() == 0);
}