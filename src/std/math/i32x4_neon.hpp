#pragma once

#include "std/Types.h"

#include <arm_neon.h>

namespace neon {
struct i32x4 {
  i32x4() : i32x4(0) {}
  i32x4(i32 s) : i32x4(s, s, s, s) {}

  i32x4(i32 x, i32 y, i32 z, i32 w) {
    const i32 arr[4] = {x, y, z, w};
    v = vld1q_s32(arr);
  }

  i32x4 operator+(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = vaddq_s32(v, other.v);
    return ret;
  }

  i32x4 operator*(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = vmulq_s32(v, other.v);
    return ret;
  }

  i32x4 operator|(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = vorrq_s32(v, other.v);
    return ret;
  }

  i32x4 operator&(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = vandq_s32(v, other.v);
    return ret;
  }

  i32x4 &operator+=(const i32x4 &other) noexcept {
    v = vaddq_s32(v, other.v);
    return *this;
  }

  i32x4 operator>=(i32x4 other) const noexcept {
    i32x4 ret;
    ret.v = vreinterpretq_s32_u32(vcgeq_s32(v, other.v));
    return ret;
  }

  i32x4 operator<(i32x4 other) const noexcept {
    i32x4 ret;
    ret.v = vreinterpretq_s32_u32(vcltq_s32(v, other.v));
    return ret;
  }

  i32x4 operator==(i32x4 other) const noexcept {
    i32x4 ret;
    ret.v = vreinterpretq_s32_u32(vceqq_s32(v, other.v));
    return ret;
  }

  bool any() const noexcept { return vmaxvq_s32(v) != 0; }

  bool none() const noexcept { return vmaxvq_s32(v) == 0; }

  i32x4 operator~() const noexcept {
    i32x4 ret;
    ret.v = vmvnq_s32(v);
    return ret;
  }

  u8 moveMask() const noexcept {
    // Shifts MSB into bit 0
    uint32x4_t tmp = vshrq_n_u32(vreinterpretq_u32_s32(v), 31);
    // Shift bit 0 (MSB) into bit 0,1,2,3, then sum horizontally
    static const int32x4_t shift = {0, 1, 2, 3};
    return vaddvq_u32(vshlq_u32(tmp, shift));
  }

  void storeTo(i32 dst[4]) const noexcept { vst1q_s32(dst, v); }

  int32x4_t v;
};
}  // namespace neon
