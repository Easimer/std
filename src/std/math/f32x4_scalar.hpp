#pragma once

#include "std/CompilerInfo.h"
#include "std/Types.h"
#include "std/math/i32x4_scalar.hpp"

#include <math.h>
#include <string.h>

namespace scalar {

#define F32X4_ELEMWISE_OP_SCALAR(DST, LHS, OP, RHS) \
  do {                                              \
    for (u32 i = 0; i < 4; i++) {                   \
      (DST).v[i] = (LHS).v[i] OP(RHS).v[i];         \
    }                                               \
  } while (0)

#define F32X4_ELEMWISE_CMP_SCALAR(DST, LHS, OP, RHS)   \
  do {                                                 \
    for (u32 i = 0; i < 4; i++) {                      \
      (DST).v[i] = ((LHS).v[i] OP(RHS).v[i]) ? -1 : 0; \
    }                                                  \
  } while (0)

struct f32x4 {
  SN_FORCEINLINE f32x4() : f32x4(0.0f) {}
  SN_FORCEINLINE f32x4(f32 s) : f32x4(s, s, s, s) {}

  SN_FORCEINLINE f32x4(f32 x, f32 y, f32 z, f32 w) : v{x, y, z, w} {}

  SN_FORCEINLINE explicit f32x4(const i32x4 &other)
      : v{f32(other.v[0]), f32(other.v[1]), f32(other.v[2]), f32(other.v[3])} {}

  SN_FORCEINLINE i32x4 castToI32x4() const noexcept {
    i32x4 ret;
    memcpy(ret.v, v, 4 * sizeof(u32));
    return ret;
  }

  SN_FORCEINLINE i32x4 convertToI32x4() const noexcept {
    i32x4 ret;
    for (u32 i = 0; i < 4; i++) {
      ret.v[i] = (i32)v[i];
    }
    return ret;
  }

  SN_FORCEINLINE f32x4 operator+(const f32x4 &other) const noexcept {
    f32x4 ret;
    F32X4_ELEMWISE_OP_SCALAR(ret, *this, +, other);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator-(const f32x4 &other) const noexcept {
    f32x4 ret;
    F32X4_ELEMWISE_OP_SCALAR(ret, *this, -, other);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator*(const f32x4 &other) const noexcept {
    f32x4 ret;
    F32X4_ELEMWISE_OP_SCALAR(ret, *this, *, other);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator/(const f32x4 &other) const noexcept {
    f32x4 ret;
    F32X4_ELEMWISE_OP_SCALAR(ret, *this, /, other);
    return ret;
  }

  SN_FORCEINLINE f32x4 &operator+=(const f32x4 &other) noexcept {
    F32X4_ELEMWISE_OP_SCALAR(*this, *this, +, other);
    return *this;
  }

  SN_FORCEINLINE i32x4 operator<=(f32x4 other) const noexcept {
    i32x4 ret;
    F32X4_ELEMWISE_CMP_SCALAR(ret, *this, >=, other);
    return ret;
  }

  SN_FORCEINLINE i32x4 operator<(f32x4 other) const noexcept {
    i32x4 ret;
    F32X4_ELEMWISE_CMP_SCALAR(ret, *this, <, other);
    return ret;
  }

  SN_FORCEINLINE i32x4 operator>(f32x4 other) const noexcept {
    i32x4 ret;
    F32X4_ELEMWISE_CMP_SCALAR(ret, *this, >, other);
    return ret;
  }

  SN_FORCEINLINE f32x4 operator*(f32 other) const noexcept {
    f32x4 ret;
    for (u32 i = 0; i < 4; i++) {
      ret.v[i] = v[i] * other;
    }
    return ret;
  }

  SN_FORCEINLINE f32x4 wwww() const noexcept {
    f32x4 ret;
    for (u32 i = 0; i < 4; i++) {
      ret.v[i] = v[3];
    }
    return ret;
  }

  SN_FORCEINLINE f32x4 rcp() const noexcept {
    f32x4 ret;
    for (u32 i = 0; i < 4; i++) {
      ret.v[i] = 1.0f / v[i];
    }
    return ret;
  }

  SN_FORCEINLINE void loadFrom(const f32 dst[4]) noexcept {
    for (u32 i = 0; i < 4; i++) {
      v[i] = dst[i];
    }
  }

  SN_FORCEINLINE void storeTo(f32 dst[4]) const noexcept {
    for (u32 i = 0; i < 4; i++) {
      dst[i] = v[i];
    }
  }

  f32 v[4];
};

inline f32x4 blend(const f32x4 &lhs,
                   const f32x4 &rhs,
                   const i32x4 &mask) noexcept {
  f32x4 ret;
  for (u32 i = 0; i < 4; i++) {
    ret.v[i] = (mask.v[i] != 0) ? lhs.v[i] : rhs.v[i];
  }
  return ret;
}

inline f32x4 min(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  for (u32 i = 0; i < 4; i++) {
    ret.v[i] = lhs.v[i] < rhs.v[i] ? lhs.v[i] : rhs.v[i];
  }
  return ret;
}

inline f32x4 max(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  for (u32 i = 0; i < 4; i++) {
    ret.v[i] = lhs.v[i] > rhs.v[i] ? lhs.v[i] : rhs.v[i];
  }
  return ret;
}

inline f32x4 abs(const f32x4 &x) noexcept {
  f32x4 ret;
  for (u32 i = 0; i < 4; i++) {
    ret.v[i] = fabsf(x.v[i]);
  }
  return ret;
}

inline f32x4 interleaveLow(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v[0] = lhs.v[0];
  ret.v[1] = rhs.v[0];
  ret.v[2] = lhs.v[1];
  ret.v[3] = rhs.v[1];
  return ret;
}

inline f32x4 interleaveHigh(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v[0] = lhs.v[2];
  ret.v[1] = rhs.v[2];
  ret.v[2] = lhs.v[3];
  ret.v[3] = rhs.v[3];
  return ret;
}

inline f32x4 moveLowHigh(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v[0] = lhs.v[0];
  ret.v[1] = lhs.v[1];
  ret.v[2] = rhs.v[2];
  ret.v[3] = rhs.v[3];
  return ret;
}

inline f32x4 moveHighLow(const f32x4 &lhs, const f32x4 &rhs) noexcept {
  f32x4 ret;
  ret.v[0] = rhs.v[0];
  ret.v[1] = rhs.v[1];
  ret.v[2] = lhs.v[2];
  ret.v[3] = lhs.v[3];
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

}  // namespace scalar
