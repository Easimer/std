#include <std/Check.h>
#include <std/Testing.hpp>
#include <std/Vector.hpp>
#include <std/VectorUtils.hpp>

SN_TEST(Vector, defaultConstructedIsEmpty) {
  Vector<u8> s;
  CHECK(s.data == nullptr);
  CHECK(s.length == 0);
  CHECK(s.capacity == 0);
}

SN_TEST(Vector, allocatesOnFirstAppend) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Vector<u8> s;

  u8 v = 0x23;
  *append(temp, &s) = v;

  CHECK(s.capacity > 0);
  CHECK(s.data != nullptr);
  CHECK(s.length == 1);
}

SN_TEST(Vector, growsWhenOutOfSpace) {
  Arena::Scope temp = getScratch(nullptr, 0);

  u8 stk[3] = {1, 2, 3};
  Vector<u8> s = {stk, 3, 3};

  u8 v = 0x23;
  *append(temp, &s) = v;

  CHECK(s.capacity > 3);
  CHECK(s.data != nullptr);
  CHECK(s.data != stk);
  CHECK(s.length == 4);
}

SN_TEST(Vector, vectorWithInitialCapacityWorks) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Vector<u8> v = vectorWithInitialCapacity<u8>(temp, 16);

  CHECK(v.capacity >= 16);
  CHECK(v.data != nullptr);
  CHECK(v.length == 0);
}

SN_TEST(Vector, vectorWithZeroInitialCapacityWorks) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Vector<u8> v = vectorWithInitialCapacity<u8>(temp, 0);

  CHECK(v.capacity >= 0);
  CHECK(v.data != nullptr);
  CHECK(v.length == 0);
}

SN_TEST(Vector, appendAddr) {
  u8 stk[2] = {0};
  Vector<u8> v = {stk, 0, 2};

  u8 *p0 = append(nullptr, &v);
  CHECK(p0 == &stk[0]);

  u8 *p1 = append(nullptr, &v);
  CHECK(p1 == &stk[1]);
}

SN_TEST(Vector, append) {
  u8 stk[2] = {0};
  Vector<u8> v = {stk, 0, 2};

  *append(nullptr, &v) = 0xBE;
  *append(nullptr, &v) = 0xEF;

  CHECK(v[0] == 0xBE);
  CHECK(v[1] == 0xEF);
}

SN_TEST(Vector, appendVal) {
  u8 stk[2] = {0};
  Vector<u8> v = {stk, 0, 2};

  appendVal<u8>(nullptr, &v, 0xBE);
  appendVal<u8>(nullptr, &v, 0xEF);

  CHECK(v[0] == 0xBE);
  CHECK(v[1] == 0xEF);
}

SN_TEST(Vector, elemsCopiedOnGrow) {
  Arena::Scope temp = getScratch(nullptr, 0);
  Vector<u8> v = vectorWithInitialCapacity<u8>(temp, 2);

  u8 *buf0 = v.data;
  u32 cap0 = v.capacity;
  while (v.length < cap0) {
    appendVal<u8>(temp, &v, 0x55);
  }

  appendVal<u8>(temp, &v, 0xFF);
  u8 *buf1 = v.data;

  // Assert that growth has happened
  CHECK(v.capacity != cap0);
  CHECK(v.data != buf0);

  // Check elements
  for (u32 i = 0; i < cap0; i++) {
    CHECK(v[i] == 0x55);
  }

  CHECK(v[cap0] == 0xFF);
}

SN_TEST(Vector, makeVectorFrom) {
  Arena::Scope temp = getScratch(nullptr, 0);
  u8 elems[3] = {1, 2, 3};

  Slice<u8> s = sliceFrom(elems);
  Vector<u8> v = makeVectorFrom(temp, s);

  CHECK(v.data != s.data);
  CHECK(v.length == s.length);
  CHECK(v.capacity >= s.length);

  CHECK(v[0] == elems[0]);
  CHECK(v[1] == elems[1]);
  CHECK(v[2] == elems[2]);
}

SN_TEST(Vector, copyToSlice) {
  Arena::Scope temp = getScratch(nullptr, 0);
  u8 elems[4] = {1, 2, 3, 4};

  Vector<u8> v = {elems, 3, 4};

  Slice<u8> s = copyToSlice(temp, v);
  CHECK(s.data != v.data);
  CHECK(s.length == v.length);

  CHECK(s[0] == elems[0]);
  CHECK(s[1] == elems[1]);
  CHECK(s[2] == elems[2]);
}

SN_TEST(Vector, sliceFrom) {
  u8 elems[4] = {1, 2, 3, 4};

  Vector<u8> v = {elems, 3, 4};

  Slice<u8> s = sliceFrom(v);
  CHECK(s.data == v.data);
  CHECK(s.length == v.length);
}
