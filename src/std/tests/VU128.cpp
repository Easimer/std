#include "std/VU128.h"
#include "std/Testing.hpp"

#include <string.h>

static const u8 bufEncoded0[5] = {0, 0, 0, 0, 0};
static const u8 bufEncoded126[5] = {126, 0, 0, 0, 0};
static const u8 bufEncoded255[5] = {240, 255, 0, 0, 0};
static const u8 bufEncoded23456[5] = {241, 160, 91, 0, 0};
static const u8 bufEncoded0xFFFFFF[5] = {242, 255, 255, 255, 0};
static const u8 bufEncoded0xFFFFFFFF[5] = {243, 255, 255, 255, 255};

SN_TEST(VU128, encode_0) {
  u8 buf[5];
  u32 lenBuf = vu128_encode(buf, 0);
  CHECK(lenBuf == 1);
  CHECK(memcmp(buf, bufEncoded0, lenBuf) == 0);
}

SN_TEST(VU128, begin_decode_0) {
  u32 sizAdditional = vu128_begin_decode(bufEncoded0);
  CHECK(sizAdditional == 0);
}

SN_TEST(VU128, decode_0) {
  u32 res = vu128_decode(bufEncoded0);
  CHECK(res == 0);
}

SN_TEST(VU128, encode_126) {
  u8 buf[5];
  u32 lenBuf = vu128_encode(buf, 126);
  CHECK(lenBuf == 1);
  CHECK(memcmp(buf, bufEncoded126, lenBuf) == 0);
}

SN_TEST(VU128, begin_decode_126) {
  u32 sizAdditional = vu128_begin_decode(bufEncoded126);
  CHECK(sizAdditional == 0);
}

SN_TEST(VU128, decode_126) {
  u32 res = vu128_decode(bufEncoded126);
  CHECK(res == 126);
}

SN_TEST(VU128, encode_255) {
  u8 buf[5];
  u32 lenBuf = vu128_encode(buf, 255);
  CHECK(lenBuf == 2);
  CHECK(memcmp(buf, bufEncoded255, lenBuf) == 0);
}

SN_TEST(VU128, begin_decode_255) {
  u32 sizAdditional = vu128_begin_decode(bufEncoded255);
  CHECK(sizAdditional == 1);
}

SN_TEST(VU128, decode_255) {
  u32 res = vu128_decode(bufEncoded255);
  CHECK(res == 255);
}

SN_TEST(VU128, encode_23456) {
  u8 buf[5];
  u32 lenBuf = vu128_encode(buf, 23456);
  CHECK(lenBuf == 3);
  CHECK(memcmp(buf, bufEncoded23456, lenBuf) == 0);
}

SN_TEST(VU128, begin_decode_23456) {
  u32 sizAdditional = vu128_begin_decode(bufEncoded23456);
  CHECK(sizAdditional == 2);
}

SN_TEST(VU128, decode_23456) {
  u32 res = vu128_decode(bufEncoded23456);
  CHECK(res == 23456);
}

SN_TEST(VU128, encode_0xFFFFFF) {
  u8 buf[5];
  u32 lenBuf = vu128_encode(buf, 0xFFFFFF);
  CHECK(lenBuf == 4);
  CHECK(memcmp(buf, bufEncoded0xFFFFFF, lenBuf) == 0);
}

SN_TEST(VU128, begin_decode_0xFFFFFF) {
  u32 sizAdditional = vu128_begin_decode(bufEncoded0xFFFFFF);
  CHECK(sizAdditional == 3);
}

SN_TEST(VU128, decode_0xFFFFFF) {
  u32 res = vu128_decode(bufEncoded0xFFFFFF);
  CHECK(res == 0xFFFFFF);
}

SN_TEST(VU128, encode_0xFFFFFFFF) {
  u8 buf[5];
  u32 lenBuf = vu128_encode(buf, 0xFFFFFFFF);
  CHECK(lenBuf == 5);
  CHECK(memcmp(buf, bufEncoded0xFFFFFFFF, lenBuf) == 0);
}

SN_TEST(VU128, begin_decode_0xFFFFFFFF) {
  u32 sizAdditional = vu128_begin_decode(bufEncoded0xFFFFFFFF);
  CHECK(sizAdditional == 4);
}

SN_TEST(VU128, decode_0xFFFFFFFF) {
  u32 res = vu128_decode(bufEncoded0xFFFFFFFF);
  CHECK(res == 0xFFFFFFFF);
}
