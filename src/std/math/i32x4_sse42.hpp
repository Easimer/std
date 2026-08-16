#pragma once

#include "std/Types.h"

#include <emmintrin.h>
#include <smmintrin.h>

namespace sse42 {
struct i32x4 {
  i32x4() : i32x4(0) {}
  i32x4(i32 s) : i32x4(s, s, s, s) {}

  i32x4(i32 x, i32 y, i32 z, i32 w) : v(_mm_setr_epi32(x, y, z, w)) {}

  i32x4 operator+(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = _mm_add_epi32(v, other.v);
    return ret;
  }

  i32x4 operator*(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = _mm_mullo_epi32(v, other.v);
    return ret;
  }

  i32x4 operator|(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = _mm_or_si128(v, other.v);
    return ret;
  }

  i32x4 operator&(const i32x4 &other) const noexcept {
    i32x4 ret;
    ret.v = _mm_and_si128(v, other.v);
    return ret;
  }

  i32x4 &operator+=(const i32x4 &other) noexcept {
    v = _mm_add_epi32(v, other.v);
    return *this;
  }

  i32x4 operator>=(i32x4 other) const noexcept {
    i32x4 ret;
    // (v >= other.v) === !(v < other.v)
    ret.v = _mm_cmplt_epi32(v, other.v);
    ret = ~ret;
    return ret;
  }

  i32x4 operator<(i32x4 other) const noexcept {
    i32x4 ret;
    ret.v = _mm_cmplt_epi32(v, other.v);
    return ret;
  }

  i32x4 operator==(i32x4 other) const noexcept {
    i32x4 ret;
    ret.v = _mm_cmpeq_epi32(v, other.v);
    return ret;
  }

  bool any() const noexcept {
    // FIXME(danielm): scalarized on WASM128
    return !_mm_test_all_zeros(v, v);
  }

  bool none() const noexcept {
    // FIXME(danielm): scalarized on WASM128
    return _mm_test_all_zeros(v, v);
  }

  i32x4 operator~() const noexcept {
    i32x4 ret;
    // NOTE(danielm): _mm_undefined_si128 doesn't exist on MSVC
    const __m128i dontCare = _mm_setzero_si128();
    const __m128i allOnes = _mm_cmpeq_epi32(dontCare, dontCare);
    ret.v = _mm_xor_si128(allOnes, v);
    return ret;
  }

  u8 moveMask() const noexcept {
    u8 ret;
    ret = u8(u32(_mm_movemask_ps(_mm_castsi128_ps(v))) & 0xFF);
    return ret;
  }

  void storeTo(i32 dst[4]) const noexcept {
    _mm_storeu_si128((__m128i *)dst, v);
  }

  __m128i v;
};
}  // namespace sse42
