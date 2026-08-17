#pragma once

#include "std/CompilerInfo.h"
#include "std/Types.h"
#include "std/math/i32x4_neon.hpp"

#include <arm_neon.h>

namespace neon {

struct f32x4 {
  SN_FORCEINLINE f32x4() : v(vdupq_n_f32(0.0f)) {}
  SN_FORCEINLINE f32x4(f32 s) : f32x4(s, s, s, s) {}

  SN_FORCEINLINE f32x4(f32 x, f32 y, f32 z, f32 w) {
    const f32 arr[4] = {x, y, z, w};
    v = vld1q_f32(arr);
  }

  SN_FORCEINLINE explicit f32x4(const i32x4 &other)
      : v(vcvtq_f32_s32(other.v)) {}

  SN_FORCEINLINE i32x4 castToI32x4() const noexcept {
    i32x4 ret;
    ret.v = vreinterpretq_s32_f32(v);
    return ret;
  }

  SN_FORCEINLINE i32x4 convertToI32x4() const noexcept {
    i32x4 ret;
    ret.v = vcvtq_s32_f32(v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator+(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = vaddq_f32(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator-(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = vsubq_f32(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator*(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = vmulq_f32(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator/(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = vdivq_f32(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 &operator+=(const f32x4 &other) noexcept {
    v = vaddq_f32(v, other.v);
    return *this;
  }

  SN_FORCEINLINE i32x4 operator<=(f32x4 other) const noexcept {
    i32x4 ret;
    ret.v = vcleq_f32(v, other.v);
    return ret;
  }

  SN_FORCEINLINE i32x4 operator<(f32x4 other) const noexcept {
    i32x4 ret;
    ret.v = vcltq_f32(v, other.v);
    return ret;
  }

  SN_FORCEINLINE i32x4 operator>(f32x4 other) const noexcept {
    i32x4 ret;
    ret.v = vcgtq_f32(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator*(f32 other) const noexcept {
    f32x4 ret;
    ret.v = vmulq_n_f32(v, other);
    return ret;
  }

  SN_FORCEINLINE f32x4 wwww() const noexcept {
    f32x4 ret;
    ret.v = vdupq_laneq_f32(v, 3);
    return ret;
  }

  SN_FORCEINLINE f32x4 rcp() const noexcept {
    f32x4 ret;
    ret.v = vrecpeq_f32(v);
    return ret;
  }

  SN_FORCEINLINE void loadFrom(const f32 dst[4]) noexcept {
    v = vld1q_f32(dst);
  }

  SN_FORCEINLINE void storeTo(f32 dst[4]) const noexcept {
    vst1q_f32(dst, v);
  }

  float32x4_t v;
};

inline f32x4 blend(const f32x4 &lhs,
                    const f32x4 &rhs,
                    const i32x4 &mask) noexcept {
  f32x4 ret;
  ret.v = vbslq_f32(vreinterpretq_u32_s32(mask.v), lhs.v, rhs.v);
  return ret;
}

inline f32x4 min(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = vminq_f32(lhs.v, rhs.v);
  return ret;
}

inline f32x4 max(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = vmaxq_f32(lhs.v, rhs.v);
  return ret;
}

inline f32x4 abs(const f32x4 &x) noexcept {
  f32x4 ret;
  ret.v = vabsq_f32(x.v);
  return ret;
}

inline f32x4 interleaveLow(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = vzip1q_f32(lhs.v, rhs.v);
  return ret;
}

inline f32x4 interleaveHigh(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = vzip2q_f32(lhs.v, rhs.v);
  return ret;
}

inline f32x4 moveLowHigh(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = vcombine_f32(vget_low_f32(lhs.v), vget_high_f32(rhs.v));
  return ret;
}

inline f32x4 moveHighLow(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = vcombine_f32(vget_low_f32(rhs.v), vget_high_f32(lhs.v));
  return ret;
}

inline void transpose(f32x4 &row0,
                      f32x4 &row1,
                      f32x4 &row2,
                      f32x4 &row3) noexcept {
  float32x4x2_t t0 = vzipq_f32(row0.v, row1.v);
  float32x4x2_t t1 = vzipq_f32(row2.v, row3.v);
  row0.v = vcombine_f32(vget_low_f32(t0.val[0]), vget_low_f32(t1.val[0]));
  row1.v = vcombine_f32(vget_high_f32(t0.val[0]), vget_high_f32(t1.val[0]));
  row2.v = vcombine_f32(vget_low_f32(t0.val[1]), vget_low_f32(t1.val[1]));
  row3.v = vcombine_f32(vget_high_f32(t0.val[1]), vget_high_f32(t1.val[1]));
}
}  // namespace neon
