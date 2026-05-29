#include "std/VU128.h"
#include "std/Types.h"

u32 vu128_encode(SN_STD_OUT_WRITES_TO(5, return ) u8 buf[5], const u32 x) {
  if (x < 0xF0) {
    buf[0] = x & 0xFF;
    return 1;
  }

  u8 *t = buf + 1;
  t[0] = (x >> 0) & 0xFF;
  t[1] = (x >> 8) & 0xFF;
  t[2] = (x >> 16) & 0xFF;
  t[3] = (x >> 24) & 0xFF;

  u8 len = (u8)(countLeadingZeros(x) / 8) ^ 0x03;
  buf[0] = 0xF0 | len;
  return len + 2;
}

u32 vu128_begin_decode(const u8 buf[1]) {
  if (buf[0] < 0xF0) {
    return 0;
  }

  return (buf[0] & 0x0F) + 1;
}

u32 vu128_decode(const u8 buf[5]) {
  if (buf[0] < 0xF0) {
    return buf[0];
  }
  u32 x = buf[4];
  x = (x << 8) | buf[3];
  x = (x << 8) | buf[2];
  x = (x << 8) | buf[1];
  u8 len = buf[0] & 0x0F;

  u8 shift = (8 * (3 - (len & 3)));
  u32 mask = 0xFFFFFFFF >> shift;
  u32 decoded = x & mask;
  return decoded;
}
