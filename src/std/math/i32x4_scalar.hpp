#pragma once

#include "std/Types.h"

namespace scalar {

#define I32X4_ELEMWISE_OP_SCALAR(DST, LHS, OP, RHS) \
  do {                                              \
    for (u32 i = 0; i < 4; i++) {                   \
      (DST).v[i] = (LHS).v[i] OP(RHS).v[i];         \
    }                                               \
  } while (0)

#define I32X4_ELEMWISE_CMP_SCALAR(DST, LHS, OP, RHS)   \
  do {                                                 \
    for (u32 i = 0; i < 4; i++) {                      \
      (DST).v[i] = ((LHS).v[i] OP(RHS).v[i]) ? -1 : 0; \
    }                                                  \
  } while (0)

struct i32x4 {
  i32x4() : i32x4(0) {}
  i32x4(i32 s) : i32x4(s, s, s, s) {}

  i32x4(i32 x, i32 y, i32 z, i32 w) : v{x, y, z, w} {}

  i32x4 operator+(const i32x4 &other) const noexcept {
    i32x4 ret;
    I32X4_ELEMWISE_OP_SCALAR(ret, *this, +, other);
    return ret;
  }

  i32x4 operator*(const i32x4 &other) const noexcept {
    i32x4 ret;
    I32X4_ELEMWISE_OP_SCALAR(ret, *this, *, other);
    return ret;
  }

  i32x4 operator|(const i32x4 &other) const noexcept {
    i32x4 ret;
    I32X4_ELEMWISE_OP_SCALAR(ret, *this, |, other);
    return ret;
  }

  i32x4 operator&(const i32x4 &other) const noexcept {
    i32x4 ret;
    I32X4_ELEMWISE_OP_SCALAR(ret, *this, &, other);
    return ret;
  }

  i32x4 &operator+=(const i32x4 &other) noexcept {
    I32X4_ELEMWISE_OP_SCALAR(*this, *this, +, other);
    return *this;
  }

  i32x4 operator>=(i32x4 other) const noexcept {
    i32x4 ret;
    I32X4_ELEMWISE_OP_SCALAR(ret, *this, >=, other);
    return ret;
  }

  i32x4 operator<(i32x4 other) const noexcept {
    i32x4 ret;
    I32X4_ELEMWISE_CMP_SCALAR(ret, *this, <, other);
    return ret;
  }

  i32x4 operator==(i32x4 other) const noexcept {
    i32x4 ret;
    I32X4_ELEMWISE_CMP_SCALAR(ret, *this, ==, other);
    return ret;
  }

  bool any() const noexcept {
    for (u32 i = 0; i < 4; i++) {
      if (v[i] != 0) {
        return true;
      }
    }

    return false;
  }

  bool none() const noexcept {
    for (u32 i = 0; i < 4; i++) {
      if (v[i] != 0) {
        return false;
      }
    }

    return true;
  }

  i32x4 operator~() const noexcept {
    i32x4 ret;
    for (u32 i = 0; i < 4; i++) {
      ret.v[i] = v[i] ^ (-1);
    }
    return ret;
  }

  u8 moveMask() const noexcept {
    u8 ret;
    // Since v contains signed integers, the shift will broadcast the sign bit
    // into every other bit. The AND then selects the appropriate bit for the
    // given lane.
    ret = ((v[0] >> 31) & 1) | ((v[1] >> 31) & 2) | ((v[2] >> 31) & 4) |
          ((v[3] >> 31) & 8);
    /*
    // To test on godbolt that the SSE2 impl and this produce the same results:
    int m0 = _mm_movemask_ps(_mm_setr_epi32(-1, 0, 0, 0));
    int v[4] = {-1, 0, 0, 0};
    int m1 = ((v[0] >> 31) & 1) | ((v[1] >> 31) & 2) | ((v[2] >> 31) & 4) |
    ((v[3]
    >> 31) & 8);
    return m0 == m1;
    */

    return ret;
  }

  void storeTo(i32 dst[4]) const noexcept {
    for (u32 i = 0; i < 4; i++) {
      dst[i] = v[i];
    }
  }

  i32 v[4];
};
}  // namespace scalar
