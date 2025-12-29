#include <std/Check.h>
#include <std/Slice.hpp>
#include <std/SliceUtils.hpp>
#include <std/Testing.hpp>

SN_TEST(Slice, defaultConstructedIsEmpty) {
  Slice<u8> s;
  CHECK(s.data == nullptr);
  CHECK(s.length == 0);
}

SN_TEST(Slice, empty) {
  Slice<u8> s = {nullptr, 0};

  CHECK(s.empty());
}

SN_TEST(Slice, notEmpty) {
  u8 value = 0;
  Slice<u8> s = {&value, 1};

  CHECK(!s.empty());
}

static const u32 fiveValues[5] = {0, 1, 2, 3, 4};
SN_TEST(Slice, indexingOperator) {
  Slice<const u32> s = {fiveValues, 5};

  for (u32 i = 0; i < 5; i++) {
    CHECK(s[i] == fiveValues[i]);
  }
}

SN_TEST(Slice, sliceFromArrayWorks) {
  Slice<const u32> s = sliceFrom(fiveValues);
  CHECK(s.data == fiveValues);
  CHECK(s.length == 5);
}

SN_TEST_MUST_FAIL(Slice, indexingOperatorAssertsWhenOob) {
  Slice<const u32> s = sliceFrom(fiveValues);

  s[666];
}

SN_TEST(Slice, indexOfSucceeds) {
  Slice<const u32> s = sliceFrom(fiveValues);

  u32 idx;
  bool res = s.indexOf(3, &idx);
  CHECK(res);
  CHECK(idx == 3);
}

SN_TEST(Slice, indexOfFails) {
  Slice<const u32> s = sliceFrom(fiveValues);

  u32 idx;
  bool res = s.indexOf(666, &idx);
  CHECK(!res);
}

SN_TEST(Slice, indexOfFindsFirst) {
  const u32 values[5] = {0, 5, 0, 5, 0};
  Slice<const u32> s = sliceFrom(values);

  u32 idx;
  bool res = s.indexOf(5, &idx);
  CHECK(res);
  CHECK(idx == 1);
}

SN_TEST(Slice, lastIndexOfSucceeds) {
  Slice<const u32> s = sliceFrom(fiveValues);

  u32 idx;
  bool res = s.indexOf(1, &idx);
  CHECK(res);
  CHECK(idx == 1);
}

SN_TEST(Slice, lastIndexOfFindsLast) {
  const u32 values[5] = {0, 5, 0, 5, 0};
  Slice<const u32> s = sliceFrom(values);

  u32 idx;
  bool res = s.lastIndexOf(5, &idx);
  CHECK(res);
  CHECK(idx == 3);
}

SN_TEST(Slice, anySucceeds) {
  Slice<const u32> s = sliceFrom(fiveValues);

  bool res = s.any([](u32 value) { return value == 3; });
  CHECK(res);
}

SN_TEST(Slice, anySucceedsWithIndex) {
  Slice<const u32> s = sliceFrom(fiveValues);

  u32 idx;
  bool res = s.any([](u32 value) { return value == 3; }, idx);
  CHECK(res);
  CHECK(idx == 3);
}

SN_TEST(Slice, allSucceeds) {
  Slice<const u32> s = sliceFrom(fiveValues);

  bool res = s.all([](u32 val) { return val < 100; });
  CHECK(res);
}

SN_TEST(Slice, allFailsSome) {
  Slice<const u32> s = sliceFrom(fiveValues);

  bool res = s.all([](u32 val) { return val < 2; });
  CHECK(!res);
}

SN_TEST(Slice, allFailsEvery) {
  Slice<const u32> s = sliceFrom(fiveValues);

  bool res = s.all([](u32 val) { return val > 10; });
  CHECK(!res);
}

SN_TEST(Slice, subarraySucceeds) {
  Slice<const u32> s = sliceFrom(fiveValues);

  Slice<const u32> sub = s.subarray(0, 1);
  CHECK(sub.data == s.data);
  CHECK(sub.length == 1);
}

SN_TEST(Slice, subarraySucceeds2) {
  Slice<const u32> s = sliceFrom(fiveValues);

  Slice<const u32> sub = s.subarray(1, 2);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 1);
}

SN_TEST(Slice, subarrayStartOob) {
  Slice<const u32> s = sliceFrom(fiveValues);

  Slice<const u32> sub = s.subarray(100, 101);
  CHECK(sub.empty());
}

SN_TEST(Slice, subarrayLenZero) {
  Slice<const u32> s = sliceFrom(fiveValues);

  Slice<const u32> sub = s.subarray(0, 0);
  CHECK(sub.empty());
}

SN_TEST(Slice, subarraySwappedIndices) {
  Slice<const u32> s = sliceFrom(fiveValues);

  Slice<const u32> sub = s.subarray(1, 0);
  CHECK(sub.empty());
}

SN_TEST(Slice, subarraySpan) {
  Slice<const u32> s = sliceFrom(fiveValues);
  Span<u32> span = {};
  span.start = 1;
  span.count = 2;

  Slice<const u32> sub = s.subarray(span);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 2);
}

SN_TEST(Slice, subarrayRange) {
  Slice<const u32> s = sliceFrom(fiveValues);
  Range<u32> range = {};
  range.start = 1;
  range.end = 3;

  Slice<const u32> sub = s.subarray(range);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 2);
}

SN_TEST(Slice, eqSucceeds) {
  const f32 values0[3] = {7, 3, 11};
  const f32 values1[3] = {7, 3, 11};
  Slice<const f32> left = sliceFrom(values0);
  Slice<const f32> right = sliceFrom(values1);
  CHECK(left == right);
}

SN_TEST(Slice, eqSucceedsSameBuffer) {
  Slice<const u32> left = sliceFrom(fiveValues);
  Slice<const u32> right = sliceFrom(fiveValues);
  CHECK(left == right);
}

SN_TEST(Slice, eqFails) {
  const u32 values1[5] = {9, 3, 7, 1, 2};
  Slice<const u32> left = sliceFrom(fiveValues);
  Slice<const u32> right = sliceFrom(values1);
  CHECK(left != right);
}

SN_TEST(Slice, eqSucceedsEmpty) {
  Slice<const u32> left = {};
  Slice<const u32> right = {};
  CHECK(left == right);
}

SN_TEST(Slice, reverseEven) {
  u32 elems[4] = {0, 1, 2, 3};
  const u32 expected[4] = {3, 2, 1, 0};
  Slice<const u32> e = sliceFrom(expected);

  Slice<u32> s = sliceFrom(elems);
  s.reverse();

  CHECK(s.asConst() == e);
}

SN_TEST(Slice, reverseOdd) {
  u32 elems[5] = {0, 1, 2, 3, 4};
  const u32 expected[5] = {4, 3, 2, 1, 0};
  Slice<const u32> e = sliceFrom(expected);

  Slice<u32> s = sliceFrom(elems);
  s.reverse();

  CHECK(s.asConst() == e);
}

SN_TEST(Slice, duplicate) {
  Arena::Scope temp = getScratch(nullptr, 0);

  u32 elems[5] = {0, 1, 2, 3, 4};
  Slice<u32> s = sliceFrom(elems);
  Slice<u32> s2 = duplicate(temp, s);

  CHECK(s.data != s2.data);
  CHECK(s == s2);
}

SN_TEST(Slice, fromCStr) {
  const char *str = "asd";
  Slice<const char> s = fromCStr(str);
  CHECK(s.data == str);
  CHECK(s.length == 3);
}

SN_TEST(Slice, fromCStrWithZero) {
  const char *str = "asd";
  Slice<const char> s = fromCStrWithZero(str);
  CHECK(s.data == str);
  CHECK(s.length == 4);
}

SN_TEST(Slice, startsWithSucceeds) {
  u8 left[4] = {1, 2, 3, 4};
  u8 prefix[2] = {1, 2};

  CHECK(sliceFrom<const u8>(left).startsWith(sliceFrom<const u8>(prefix)));
}

SN_TEST(Slice, startsWithFails) {
  u8 left[4] = {1, 2, 3, 4};
  u8 prefix[2] = {1, 0};

  CHECK(!sliceFrom<const u8>(left).startsWith(sliceFrom<const u8>(prefix)));
}

SN_TEST(Slice, startsWithSucceedsEmpty) {
  u8 left[4] = {1, 2, 3, 4};

  Slice<const u8> sleft = sliceFrom<const u8>(left);
  Slice<const u8> empty;
  CHECK(sleft.startsWith(empty));
  CHECK(empty.startsWith(empty));
  CHECK(!empty.startsWith(sleft));
}

SN_TEST(Slice, zeroMemory) {
  u8 left[4] = {1, 2, 3, 4};

  zeroMemory(sliceFrom(left));
  for (u32 i = 0; i < 4; i++) {
    CHECK(left[i] == 0);
  }
}

SN_TEST(Slice, copy) {
  u8 dst[4] = {1, 2, 3, 4};
  const u8 src[4] = {8, 9, 10, 11};

  copy(sliceFrom(dst), sliceFrom(src));
  for (u32 i = 0; i < 4; i++) {
    CHECK(dst[i] == src[i]);
  }
}

SN_TEST(Slice, copyPartial) {
  u8 dst[4] = {1, 2, 3, 4};
  const u8 src[2] = {8, 9};

  copy(sliceFrom(dst), sliceFrom(src));
  CHECK(dst[0] == src[0]);
  CHECK(dst[1] == src[1]);
  CHECK(dst[2] == 3);
  CHECK(dst[3] == 4);
}

SN_TEST(Slice, concat) {
  Arena::Scope temp = getScratch(nullptr, 0);
  const u8 left[2] = {1, 2};
  const u8 right[2] = {3, 4};

  Slice<u8> result = concat(temp, sliceFrom(left), sliceFrom(right));
  CHECK(result[0] == left[0]);
  CHECK(result[1] == left[1]);
  CHECK(result[2] == right[0]);
  CHECK(result[3] == right[1]);
}

SN_TEST(Slice, concatZeroTerminate) {
  Arena::Scope temp = getScratch(nullptr, 0);
  const char left[4] = "hey";
  const char right[3] = "ho";

  Slice<char> result = concatZeroTerminate(temp, fromCStr(left), fromCStr(right));
  CHECK(result.length == 6);
  CHECK(strcmp(result.data, "heyho") == 0);
}

SN_TEST(Slice, endsWithSucceeds) {
  u8 left[4] = {1, 2, 3, 4};
  u8 suffix[2] = {3, 4};

  CHECK(sliceFrom<const u8>(left).endsWith(sliceFrom<const u8>(suffix)));
}

SN_TEST(Slice, endsWithFails) {
  u8 left[4] = {1, 2, 3, 4};
  u8 suffix[2] = {3, 0};

  CHECK(!sliceFrom<const u8>(left).endsWith(sliceFrom<const u8>(suffix)));
}

SN_TEST(Slice, endsWithSucceedsEmpty) {
  u8 left[4] = {1, 2, 3, 4};

  Slice<const u8> sleft = sliceFrom<const u8>(left);
  Slice<const u8> empty;
  CHECK(sleft.endsWith(empty));
  CHECK(empty.endsWith(empty));
  CHECK(!empty.endsWith(sleft));
}

SN_TEST(Slice, fill) {
  i32 dst[3] = {5, 11, 3};

  sliceFrom(dst).fill(7);
  CHECK(dst[0] == 7);
  CHECK(dst[1] == 7);
  CHECK(dst[2] == 7);
}

SN_TEST(Slice, copyWithConversionTo) {
  const f32 src[3] = {1.0f, 2.0f, 3.0f};
  i32 dst[3];

  sliceFrom(src).copyWithConversionTo(sliceFrom(dst));
  CHECK(dst[0] == 1);
  CHECK(dst[1] == 2);
  CHECK(dst[2] == 3);
}

SN_TEST(Slice, shrinkFromLeftByCount1) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(1);
  CHECK(s.length == 2);
  CHECK(s.data == dst+1);
}

SN_TEST(Slice, shrinkFromLeftByCount2) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(2);
  CHECK(s.length == 1);
  CHECK(s.data == dst+2);
}

SN_TEST(Slice, shrinkFromLeftByCount3) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(3);
  CHECK(s.empty());
}

SN_TEST(Slice, copyU32) {
  const u32 src[3] = {5, 11, 3};
  u32 dst[3];
  memset(dst, 0, sizeof(dst));

  sliceFrom(dst).copy(sliceFrom(src));

  for (u32 i = 0; i < 3; i++) {
    CHECK(src[i] == dst[i]);
  }
}
