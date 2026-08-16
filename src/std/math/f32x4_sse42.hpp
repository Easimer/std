#pragma once

#include "std/CompilerInfo.h"
#include "std/Types.h"
#include "std/math/i32x4_sse42.hpp"

#include <smmintrin.h>

namespace sse42 {

struct f32x4 {
  SN_FORCEINLINE f32x4() : v(_mm_setzero_ps()) {}
  SN_FORCEINLINE f32x4(f32 s) : f32x4(s, s, s, s) {}

  SN_FORCEINLINE f32x4(f32 x, f32 y, f32 z, f32 w)
      : v(_mm_setr_ps(x, y, z, w)) {}

  SN_FORCEINLINE explicit f32x4(const i32x4 &other)
      : v(_mm_cvtepi32_ps(other.v)) {}

  SN_FORCEINLINE i32x4 castToI32x4() const noexcept {
    i32x4 ret;
    ret.v = _mm_castps_si128(v);
    return ret;
  }

  SN_FORCEINLINE i32x4 convertToI32x4() const noexcept {
    i32x4 ret;
    ret.v = _mm_cvtps_epi32(v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator+(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = _mm_add_ps(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator-(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = _mm_sub_ps(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator*(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = _mm_mul_ps(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator/(const f32x4 &other) const noexcept {
    f32x4 ret;
    ret.v = _mm_div_ps(v, other.v);
    return ret;
  }

  SN_FORCEINLINE f32x4 &operator+=(const f32x4 &other) noexcept {
    v = _mm_add_ps(v, other.v);
    return *this;
  }

  SN_FORCEINLINE i32x4 operator<=(f32x4 other) const noexcept {
    i32x4 ret;
    // (v <= other.v) === !(v > other.v)
    ret.v = _mm_castps_si128(_mm_cmpgt_ps(v, other.v));
    ret = ~ret;
    return ret;
  }

  SN_FORCEINLINE i32x4 operator<(f32x4 other) const noexcept {
    i32x4 ret;
    ret.v = _mm_castps_si128(_mm_cmplt_ps(v, other.v));
    return ret;
  }

  SN_FORCEINLINE i32x4 operator>(f32x4 other) const noexcept {
    i32x4 ret;
    ret.v = _mm_castps_si128(_mm_cmpgt_ps(v, other.v));
    return ret;
  }

  SN_FORCEINLINE f32x4 operator*(f32 other) const noexcept {
    f32x4 ret;
    ret.v = _mm_mul_ps(v, _mm_set1_ps(other));
    return ret;
  }

  SN_FORCEINLINE f32x4 wwww() const noexcept {
    f32x4 ret;
    ret.v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
    return ret;
  }

  SN_FORCEINLINE f32x4 rcp() const noexcept {
    f32x4 ret;
    ret.v = _mm_rcp_ps(v);
    return ret;
  }

  SN_FORCEINLINE void loadFrom(const f32 dst[4]) noexcept {
    v = _mm_loadu_ps(dst);
  }

  SN_FORCEINLINE void storeTo(f32 dst[4]) const noexcept {
    _mm_storeu_ps(dst, v);
  }

  __m128 v;
};

inline f32x4 blend(const f32x4 &lhs,
                   const f32x4 &rhs,
                   const i32x4 &mask) noexcept {
  f32x4 ret;
  ret.v = _mm_blendv_ps(rhs.v, lhs.v, _mm_castsi128_ps(mask.v));
  return ret;
}

inline f32x4 min(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = _mm_min_ps(lhs.v, rhs.v);
  return ret;
}

inline f32x4 max(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = _mm_max_ps(lhs.v, rhs.v);
  return ret;
}

inline f32x4 abs(const f32x4 &x) noexcept {
  f32x4 ret;
  const __m128 sign_mask = _mm_set1_ps(-0.f);
  ret.v = _mm_andnot_ps(sign_mask, x.v);
  return ret;
}

inline f32x4 interleaveLow(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = _mm_unpacklo_ps(lhs.v, rhs.v);
  return ret;
}

inline f32x4 interleaveHigh(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = _mm_unpackhi_ps(lhs.v, rhs.v);
  return ret;
}

inline f32x4 moveLowHigh(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = _mm_movelh_ps(lhs.v, rhs.v);
  return ret;
}

inline f32x4 moveHighLow(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v = _mm_movehl_ps(lhs.v, rhs.v);
  return ret;
}

inline void transpose(f32x4 &row0,
                      f32x4 &row1,
                      f32x4 &row2,
                      f32x4 &row3) noexcept {
  f32x4 t0 = interleaveLow(row0, row1);
  f32x4 t1 = interleaveLow(row2, row3);
  f32x4 t2 = interleaveHigh(row0, row1);
  f32x4 t3 = interleaveHigh(row2, row3);
  row0 = moveLowHigh(t0, t1);
  row1 = moveHighLow(t1, t0);
  row2 = moveLowHigh(t2, t3);
  row3 = moveHighLow(t3, t2);
}
}  // namespace sse42
