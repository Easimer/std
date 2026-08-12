#include <std/Check.h>
#include <std/Slice.hpp>
#include <std/SliceUtils.hpp>
#include <std/Testing.hpp>

#include <vector>

SN_TEST(Slice, defaultConstructedIsEmpty) {
  Slice<u8> s;
  CHECK(s.data == nullptr);
  CHECK(s.length == 0);
}

SN_TEST(Slice, constructFromPointer) {
  const u8 values[3] = {1, 2, 3};
  Slice<u8> s = Slice<u8>(values, 3);
  CHECK(s.data == values);
  CHECK(s.length == 3);
}

SN_TEST(Slice, constructFromValue) {
  u32 value = 4;
  Slice<u32> s = Slice<u32>(value);
  CHECK(s.data == &value);
  CHECK(s.length == 1);
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
  Slice<u32> s = {fiveValues, 5};

  for (size_t i = 0; i < 5; i++) {
    CHECK(s[i] == fiveValues[i]);
  }
}

SN_TEST(Slice, sliceFromArrayWorks) {
  Slice<u32> s = sliceFrom(fiveValues);
  CHECK(s.data == fiveValues);
  CHECK(s.length == 5);
}

SN_TEST_MUST_FAIL(Slice, indexingOperatorAssertsWhenOob) {
  Slice<u32> s = sliceFrom(fiveValues);

  s[666];
}

SN_TEST(Slice, indexOfSucceeds) {
  Slice<u32> s = sliceFrom(fiveValues);

  u32 idx;
  bool res = s.indexOf(3, &idx);
  CHECK(res);
  CHECK(idx == 3);
}

SN_TEST(Slice, indexOfFails) {
  Slice<u32> s = sliceFrom(fiveValues);

  u32 idx;
  bool res = s.indexOf(666, &idx);
  CHECK(!res);
}

SN_TEST(Slice, indexOfFindsFirst) {
  const u32 values[5] = {0, 5, 0, 5, 0};
  Slice<u32> s = sliceFrom(values);

  u32 idx;
  bool res = s.indexOf(5, &idx);
  CHECK(res);
  CHECK(idx == 1);
}

SN_TEST(Slice, lastIndexOfSucceeds) {
  Slice<u32> s = sliceFrom(fiveValues);

  u32 idx;
  bool res = s.indexOf(1, &idx);
  CHECK(res);
  CHECK(idx == 1);
}

SN_TEST(Slice, lastIndexOfFindsLast) {
  const u32 values[5] = {0, 5, 0, 5, 0};
  Slice<u32> s = sliceFrom(values);

  u32 idx;
  bool res = s.lastIndexOf(5, &idx);
  CHECK(res);
  CHECK(idx == 3);
}

SN_TEST(Slice, anySucceeds) {
  Slice<u32> s = sliceFrom(fiveValues);

  Optional<size_t> index = s.any([](u32 value) { return value == 3; });
  CHECK(index.hasValue());
}

SN_TEST(Slice, anySucceedsWithIndex) {
  Slice<u32> s = sliceFrom(fiveValues);

  Optional<size_t> idx = s.any([](u32 value) { return value == 3; });
  CHECK(idx.hasValue());
  CHECK(idx.value() == 3);
}

SN_TEST(Slice, allSucceeds) {
  Slice<u32> s = sliceFrom(fiveValues);

  Optional<size_t> idx = s.all([](u32 val) { return val < 100; });
  CHECK(idx.hasValue());
}

SN_TEST(Slice, allFailsSome) {
  Slice<u32> s = sliceFrom(fiveValues);

  bool res = s.all([](u32 val) { return val < 2; });
  CHECK(!res);
}

SN_TEST(Slice, allFailsEvery) {
  Slice<u32> s = sliceFrom(fiveValues);

  bool res = s.all([](u32 val) { return val > 10; });
  CHECK(!res);
}

SN_TEST(Slice, subarraySucceeds) {
  Slice<u32> s = sliceFrom(fiveValues);

  Slice<u32> sub = s.subarray(0, 1);
  CHECK(sub.data == s.data);
  CHECK(sub.length == 1);
}

SN_TEST(Slice, subarraySucceeds2) {
  Slice<u32> s = sliceFrom(fiveValues);

  Slice<u32> sub = s.subarray(1, 2);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 1);
}

SN_TEST(Slice, subarrayStartOob) {
  Slice<u32> s = sliceFrom(fiveValues);

  Slice<u32> sub = s.subarray(100, 101);
  CHECK(sub.empty());
}

SN_TEST(Slice, subarrayLenZero) {
  Slice<u32> s = sliceFrom(fiveValues);

  Slice<u32> sub = s.subarray(0, 0);
  CHECK(sub.empty());
}

SN_TEST(Slice, subarraySwappedIndices) {
  Slice<u32> s = sliceFrom(fiveValues);

  Slice<u32> sub = s.subarray(1, 0);
  CHECK(sub.empty());
}

SN_TEST(Slice, subarraySpan) {
  Slice<u32> s = sliceFrom(fiveValues);
  Span<size_t> span = {};
  span.start = 1;
  span.count = 2;

  Slice<u32> sub = s.subarray(span);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 2);
}

SN_TEST(Slice, subarrayRange) {
  Slice<u32> s = sliceFrom(fiveValues);
  Range<size_t> range = {};
  range.start = 1;
  range.end = 3;

  Slice<u32> sub = s.subarray(range);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 2);
}

SN_TEST(Slice, eqSucceeds) {
  const f32 values0[3] = {7, 3, 11};
  const f32 values1[3] = {7, 3, 11};
  Slice<f32> left = sliceFrom(values0);
  Slice<f32> right = sliceFrom(values1);
  CHECK(left == right);
}

SN_TEST(Slice, eqSucceedsSameBuffer) {
  Slice<u32> left = sliceFrom(fiveValues);
  Slice<u32> right = sliceFrom(fiveValues);
  CHECK(left == right);
}

SN_TEST(Slice, eqFails) {
  const u32 values1[5] = {9, 3, 7, 1, 2};
  Slice<u32> left = sliceFrom(fiveValues);
  Slice<u32> right = sliceFrom(values1);
  CHECK(left != right);
}

SN_TEST(Slice, eqSucceedsEmpty) {
  Slice<u32> left = {};
  Slice<u32> right = {};
  CHECK(left == right);
}

SN_TEST(MutSlice, reverseEmpty) {
  MutSlice<u32> s;
  s.reverse();
}

SN_TEST(MutSlice, reverseEven) {
  u32 elems[4] = {0, 1, 2, 3};
  const u32 expected[4] = {3, 2, 1, 0};
  Slice<u32> e = sliceFrom(expected);

  MutSlice<u32> s = sliceFrom(elems);
  s.reverse();

  CHECK(s == e);
}

SN_TEST(MutSlice, reverseOdd) {
  u32 elems[5] = {0, 1, 2, 3, 4};
  const u32 expected[5] = {4, 3, 2, 1, 0};
  Slice<u32> e = sliceFrom(expected);

  MutSlice<u32> s = sliceFrom(elems);
  s.reverse();

  CHECK(s == e);
}

SN_TEST(Slice, duplicate) {
  Arena::Scope temp = getScratch(nullptr, 0);

  u32 elems[5] = {0, 1, 2, 3, 4};
  Slice<u32> s = sliceFrom(elems);
  MutSlice<u32> s2 = duplicate(temp, s);

  CHECK(s.data != s2.data);
  CHECK(s == s2);
}

SN_TEST(Slice, fromCStr) {
  const char *str = "asd";
  Slice<char> s = fromCStr(str);
  CHECK(s.data == str);
  CHECK(s.length == 3);
}

SN_TEST(Slice, fromCStrWithZero) {
  const char *str = "asd";
  Slice<char> s = fromCStrWithZero(str);
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

  Slice<u8> sleft = sliceFrom<u8>(left);
  Slice<u8> empty;
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

SN_TEST(MutSlice, copy) {
  u8 dst[4] = {1, 2, 3, 4};
  const u8 src[4] = {8, 9, 10, 11};

  sliceFrom(dst).copy(sliceFrom(src));
  for (u32 i = 0; i < 4; i++) {
    CHECK(dst[i] == src[i]);
  }
}

SN_TEST(MutSlice, copyPartial) {
  u8 dst[4] = {1, 2, 3, 4};
  const u8 src[2] = {8, 9};

  sliceFrom(dst).copy(sliceFrom(src));
  CHECK(dst[0] == src[0]);
  CHECK(dst[1] == src[1]);
  CHECK(dst[2] == 3);
  CHECK(dst[3] == 4);
}

SN_TEST(MutSlice, memcopy) {
  u8 dst[4] = {1, 2, 3, 4};
  const u8 src[4] = {8, 9, 10, 11};

  sliceFrom(dst).memcopy(sliceFrom(src));
  CHECK(dst[0] == src[0]);
  CHECK(dst[1] == src[1]);
  CHECK(dst[2] == src[2]);
  CHECK(dst[3] == src[3]);
}

SN_TEST(MutSlice, concat) {
  Arena::Scope temp = getScratch(nullptr, 0);
  const u8 left[2] = {1, 2};
  const u8 right[2] = {3, 4};

  MutSlice<u8> result = concat(temp, sliceFrom(left), sliceFrom(right));
  CHECK(result[0] == left[0]);
  CHECK(result[1] == left[1]);
  CHECK(result[2] == right[0]);
  CHECK(result[3] == right[1]);
}

SN_TEST(MutSlice, concatZeroTerminate) {
  Arena::Scope temp = getScratch(nullptr, 0);
  const char left[4] = "hey";
  const char right[3] = "ho";

  MutSlice<char> result =
      concatZeroTerminate(temp, fromCStr(left), fromCStr(right));
  CHECK(result.length == 6);
  CHECK(strcmp(result.data, "heyho") == 0);
}

SN_TEST(MutSlice, zeroTerminated) {
  Arena::Scope temp = getScratch(nullptr, 0);
  const char left[4] = "hey";

  MutSlice<char> result = zeroTerminated(temp, fromCStr(left));
  CHECK(result.length == 4);
  CHECK(result[3] == '\0');
  CHECK(strcmp(result.data, "hey") == 0);
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

  Slice<u8> sleft = sliceFrom<u8>(left);
  Slice<u8> empty;
  CHECK(sleft.endsWith(empty));
  CHECK(empty.endsWith(empty));
  CHECK(!empty.endsWith(sleft));
}

SN_TEST(MutSlice, fill) {
  i32 dst[3] = {5, 11, 3};

  sliceFrom(dst).fill(7);
  CHECK(dst[0] == 7);
  CHECK(dst[1] == 7);
  CHECK(dst[2] == 7);
}

SN_TEST(MutSlice, copyWithConversionFrom) {
  const f32 src[3] = {1.0f, 2.0f, 3.0f};
  i32 dst[3];

  sliceFrom(dst).copyWithConversionFrom(sliceFrom(src));
  CHECK(dst[0] == 1);
  CHECK(dst[1] == 2);
  CHECK(dst[2] == 3);
}

SN_TEST(Slice, shrinkFromLeftByCount1) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(1);
  CHECK(s.length == 2);
  CHECK(s.data == dst + 1);
}

SN_TEST(Slice, shrinkFromLeftByCount2) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(2);
  CHECK(s.length == 1);
  CHECK(s.data == dst + 2);
}

SN_TEST(Slice, shrinkFromLeftByCount3) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(3);
  CHECK(s.empty());
}

SN_TEST(Slice, shift) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  size_t res;
  
  res = s.shift();
  CHECK(res == 2);
  CHECK(s.length == res);

  CHECK(s[0] == 11);
  CHECK(s[1] == 3);

  res = s.shift(2);
  CHECK(res == 0);
  CHECK(s.length == 0);

  res = s.shift(3);
  CHECK(res == 0);
  CHECK(s.length == 0);
}

SN_TEST(Slice, pop) {
  i32 dst[3] = {5, 11, 3};
  Slice<i32> s = sliceFrom(dst);

  size_t res;
  
  res = s.pop();
  CHECK(res == 2);
  CHECK(s.length == res);

  CHECK(s[0] == 5);
  CHECK(s[1] == 11);

  res = s.pop(2);
  CHECK(res == 0);
  CHECK(s.length == 0);

  res = s.pop(3);
  CHECK(res == 0);
  CHECK(s.length == 0);
}

SN_TEST(MutSlice, shift) {
  i32 dst[3] = {5, 11, 3};
  MutSlice<i32> s = sliceFrom(dst);

  size_t res;
  
  res = s.shift();
  CHECK(res == 2);
  CHECK(s.length == res);

  CHECK(s[0] == 11);
  CHECK(s[1] == 3);

  res = s.shift(2);
  CHECK(res == 0);
  CHECK(s.length == 0);

  res = s.shift(3);
  CHECK(res == 0);
  CHECK(s.length == 0);
}

SN_TEST(MutSlice, pop) {
  i32 dst[3] = {5, 11, 3};
  MutSlice<i32> s = sliceFrom(dst);

  size_t res;
  
  res = s.pop();
  CHECK(res == 2);
  CHECK(s.length == res);

  CHECK(s[0] == 5);
  CHECK(s[1] == 11);

  res = s.pop(2);
  CHECK(res == 0);
  CHECK(s.length == 0);

  res = s.pop(3);
  CHECK(res == 0);
  CHECK(s.length == 0);
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

SN_TEST(Slice, equals) {
  char bufAbcd[4] = {'A', 'B', 'C', 'D'};
  char bufAbcde[5] = {'A', 'B', 'C', 'D', 'E'};
  char bufDcba[4] = {'D', 'C', 'B', 'A'};

  Slice<char> abcd = {bufAbcd, 4};
  Slice<char> abcde = {bufAbcde, 5};
  Slice<char> abcde_4 = {bufAbcde, 4};
  Slice<char> dcba = {bufDcba, 4};

  Slice<char> empty = {nullptr, 0};
  CHECK(empty == empty);
  CHECK(empty != abcd);
  CHECK(abcd != empty);
  CHECK(empty != abcde);
  CHECK(empty != abcde_4);
  CHECK(empty != dcba);

  CHECK(abcd == abcd);
  CHECK(abcd == abcde_4);
  CHECK(abcde_4 == abcd);

  CHECK(abcde == abcde);
  CHECK(abcde != abcde_4);
  CHECK(abcde_4 != abcde);

  CHECK(abcd != abcde);
  CHECK(abcd != dcba);

  CHECK(empty == empty);
  CHECK(abcd != empty);
}

SN_TEST(Slice, byteLength) {
  Slice<char> text = {"asd", 3};
  CHECK(text.length == 3);
  CHECK(text.byteLength() == 3);

  u64 qwords[3] = {5, 6, 7};
  Slice<u64> qw = {qwords, 3};
  CHECK(qw.length == 3);
  CHECK(qw.byteLength() == 3 * 8);
}

SN_TEST(Slice, cast) {
  u64 qwords[3] = {5, 6, 7};
  Slice<u64> qw = {qwords, 3};

  Slice<u8> bytesOfQw = qw.cast<u8>();
  CHECK(bytesOfQw.length == 3 * 8);

  u8 bufBytes[4] = {0xDE, 0xAD, 0xBE, 0xEF};
  Slice<u8> bytes = {bufBytes, 4};
  Slice<u32> dwordsOfBytes = bytes.cast<u32>();
  CHECK(dwordsOfBytes.length == 1);
}

SN_TEST(MutSlice, replace_empty) {
  MutSlice<char> empty;
  Slice<char> res = empty.replace(',', ' ');
  CHECK(res.empty());
}

SN_TEST(MutSlice, replace_normal) {
  Arena::Scope temp = getScratch(nullptr, 0);

  char str[] = "asd,123,wawa";
  Slice<char> expected = sliceFromConstChar("asd 123 wawa");

  MutSlice<char> input = sliceFrom(str);
  input.length -= 1;  // remove zero terminator

  Slice<char> res = input.replace(',', ' ');
  CHECK(res.data == input.data);
  CHECK(res.length == input.length);

  CHECK(res == expected);
}

SN_TEST(MutSlice, replace_nothingToReplace) {
  Arena::Scope temp = getScratch(nullptr, 0);

  char str[] = "asd";
  Slice<char> expected = sliceFromConstChar("asd");

  MutSlice<char> input = sliceFrom(str);
  input.length -= 1;  // remove zero terminator

  Slice<char> res = input.replace(',', ' ');
  CHECK(res.data == input.data);
  CHECK(res.length == input.length);

  CHECK(res == expected);
}

SN_TEST(Slice, count) {
  u32 arr[] = {1, 2, 1, 2, 1};

  Slice<u32> sarr = sliceFrom(arr);
  size_t actual = sarr.count(1);
  CHECK(actual == 3);
}

SN_TEST(Slice, countIf) {
  u32 arr[] = {1, 2, 3, 4, 5};

  Slice<u32> sarr = sliceFrom(arr);
  size_t actual = sarr.countIf([](u32 v) { return (v % 2) == 0; });
  CHECK(actual == 2);
}

SN_TEST(Span, empty) {
  Span<u32> zero = {0, 0};
  CHECK(zero.empty());

  Span<u32> zero2 = {2, 0};
  CHECK(zero2.empty());
}

SN_TEST(Span, containsEmptyInNonEmpty) {
  Span<u32> outer = {0, 4};  // 0,1,2,3,
  Span<u32> inner = {0, 0};  // ,
  CHECK(outer.contains(inner) == false);
}

SN_TEST(Span, containsNonEmptyInEmpty) {
  Span<u32> outer = {0, 0};  // ,
  Span<u32> inner = {0, 4};  // 0,1,2,3,
  CHECK(outer.contains(inner) == false);
}

SN_TEST(Span, containsLarger) {
  Span<u32> outer = {1, 2};  // 1,2,
  Span<u32> inner = {0, 4};  // 0,1,2,3,
  CHECK(outer.contains(inner) == false);
}

SN_TEST(Span, containsLeft) {
  Span<u32> outer = {2, 2};  // 2,3,
  Span<u32> inner = {0, 1};  // 0,
  CHECK(outer.contains(inner) == false);
}

SN_TEST(Span, containsLeftOverlap) {
  Span<u32> outer = {2, 2};  // 2,3,
  Span<u32> inner = {1, 2};  // 1,2,
  CHECK(outer.contains(inner) == false);
}

SN_TEST(Span, containsLeftEdge) {
  Span<u32> outer = {2, 2};  // 2,3,
  Span<u32> inner = {2, 1};  // 2,
  CHECK(outer.contains(inner) == true);
}

SN_TEST(Span, containsInside) {
  Span<u32> outer = {2, 4};  // 2,3,4,5
  Span<u32> inner = {3, 2};  // 3,4,
  CHECK(outer.contains(inner) == true);
}

SN_TEST(Span, containsRightEdge) {
  Span<u32> outer = {0, 4};  // 0,1,2,3,
  Span<u32> inner = {3, 1};  // 3,
  CHECK(outer.contains(inner) == true);
}

SN_TEST(Span, containsRightOverlap) {
  Span<u32> outer = {0, 4};  // 0,1,2,3,
  Span<u32> inner = {3, 2};  // 3,4,
  CHECK(outer.contains(inner) == false);
}

SN_TEST(Span, containsRight) {
  Span<u32> outer = {0, 4};  // 0,1,2,3,
  Span<u32> inner = {4, 1};  // 4,
  CHECK(outer.contains(inner) == false);
}

SN_TEST(MutSlice, subarraySucceeds) {
  u32 fiveValues[5];
  MutSlice<u32> s = sliceFrom(fiveValues);

  MutSlice<u32> sub = s.subarray(0, 1);
  CHECK(sub.data == s.data);
  CHECK(sub.length == 1);
}

SN_TEST(MutSlice, subarraySucceeds2) {
  u32 fiveValues[5];
  MutSlice<u32> s = sliceFrom(fiveValues);

  MutSlice<u32> sub = s.subarray(1, 2);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 1);
}

SN_TEST(MutSlice, subarrayStartOob) {
  u32 fiveValues[5];
  MutSlice<u32> s = sliceFrom(fiveValues);

  MutSlice<u32> sub = s.subarray(100, 101);
  CHECK(sub.empty());
}

SN_TEST(MutSlice, subarrayLenZero) {
  u32 fiveValues[5];
  MutSlice<u32> s = sliceFrom(fiveValues);

  MutSlice<u32> sub = s.subarray(0, 0);
  CHECK(sub.empty());
}

SN_TEST(MutSlice, subarraySwappedIndices) {
  u32 fiveValues[5];
  MutSlice<u32> s = sliceFrom(fiveValues);

  MutSlice<u32> sub = s.subarray(1, 0);
  CHECK(sub.empty());
}

SN_TEST(MutSlice, subarraySpan) {
  u32 fiveValues[5];
  MutSlice<u32> s = sliceFrom(fiveValues);
  Span<size_t> span = {};
  span.start = 1;
  span.count = 2;

  MutSlice<u32> sub = s.subarray(span);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 2);
}

SN_TEST(MutSlice, subarrayRange) {
  u32 fiveValues[5];
  MutSlice<u32> s = sliceFrom(fiveValues);
  Range<size_t> range = {};
  range.start = 1;
  range.end = 3;

  MutSlice<u32> sub = s.subarray(range);
  CHECK(sub.data == s.data + 1);
  CHECK(sub.length == 2);
}

SN_TEST(MutSlice, shrinkFromLeftByCount1) {
  i32 dst[3] = {5, 11, 3};
  MutSlice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(1);
  CHECK(s.length == 2);
  CHECK(s.data == dst + 1);
}

SN_TEST(MutSlice, shrinkFromLeftByCount2) {
  i32 dst[3] = {5, 11, 3};
  MutSlice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(2);
  CHECK(s.length == 1);
  CHECK(s.data == dst + 2);
}

SN_TEST(MutSlice, shrinkFromLeftByCount3) {
  i32 dst[3] = {5, 11, 3};
  MutSlice<i32> s = sliceFrom(dst);

  s.shrinkFromLeftByCount(3);
  CHECK(s.empty());
}

SN_TEST(Slice, fromStd) {
  const std::vector<u32> emptyVec;
  std::vector<u32> vec = {5, 6};

  Slice<u32> emptySlice = sliceFromStd(emptyVec);
  CHECK(emptySlice.empty());

  MutSlice<u32> slice = mutSliceFromStd(vec);
  CHECK(slice.data == vec.data());
  CHECK(slice.length == vec.size());

  MutSlice<u32> slice2 = sliceFromStd(vec);
  CHECK(slice2.data == vec.data());
  CHECK(slice2.length == vec.size());

  Slice<u32> slice3 = sliceFromStd(vec);
  CHECK(slice3.data == vec.data());
  CHECK(slice3.length == vec.size());
}
