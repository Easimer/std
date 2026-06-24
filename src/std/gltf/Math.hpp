#pragma once

#include "std/gltf/Descriptor.hpp"

#include <math.h>

namespace sn::gltf {

static inline Quat quatFromRotationMatrix(const f32 matrix[9]) {
  f32 r11 = matrix[0];
  f32 r21 = matrix[1];
  f32 r31 = matrix[2];

  f32 r12 = matrix[3];
  f32 r22 = matrix[4];
  f32 r32 = matrix[5];

  f32 r13 = matrix[6];
  f32 r23 = matrix[7];
  f32 r33 = matrix[8];

  auto sq = [](f32 x) { return x * x; };

  f32 q0 = 0.25f * sqrtf(sq(r11 + r22 + r33 + 1.0f) + sq(r32 - r23) +
                         sq(r13 - r31) + sq(r21 - r12));
  f32 q1 = 0.25f * sqrtf(sq(r32 - r23) + sq(r11 - r22 - r33 + 1) +
                         sq(r21 + r12) + sq(r31 + r13));
  f32 q2 = 0.25f * sqrtf(sq(r13 - r31) + sq(r21 + r12) +
                         sq(r22 - r11 - r33 + 1) + sq(r32 + r23));
  f32 q3 = 0.25f * sqrtf(sq(r21 - r12) + sq(r31 + r13) + sq(r32 + r23) +
                         sq(r33 - r11 - r22 + 1));

  q0 = fabsf(q0);
  // q0 = copysignf(q0, r11 + r22 + r33);
  q1 = copysignf(q1, r32 - r23);
  q2 = copysignf(q2, r13 - r31);
  q3 = copysignf(q3, r21 - r12);

  return Quat(q1, q2, q3, q0);
}

static inline void rotationMatrixFrom(f32 M[9], Quat Q) {
  if (Q.w < 0.0f) {
    Q.w *= -1.0f;
    Q.x *= -1.0f;
    Q.y *= -1.0f;
    Q.z *= -1.0f;
  }

  M[0] = 1.0f - 2.0f * (Q.y * Q.y + Q.z * Q.z);
  M[1] = 2.0f * (Q.x * Q.y + Q.z * Q.w);
  M[2] = 2.0f * (Q.x * Q.z - Q.y * Q.w);

  M[3] = 2.0f * (Q.x * Q.y - Q.z * Q.w);
  M[4] = 1.0f - 2.0f * (Q.x * Q.x + Q.z * Q.z);
  M[5] = 2.0f * (Q.y * Q.z + Q.x * Q.w);

  M[6] = 2.0f * (Q.x * Q.z + Q.y * Q.w);
  M[7] = 2.0f * (Q.y * Q.z - Q.x * Q.w);
  M[8] = 1.0f - 2.0f * (Q.x * Q.x + Q.y * Q.y);
}

static inline void decompose(const f32 M[16], Quat *rotation, Vec3 *scale) {
  f32 scaleX = sqrtf(M[0] * M[0] + M[1] * M[1] + M[2] * M[2]);
  f32 scaleY = sqrtf(M[4] * M[4] + M[5] * M[5] + M[6] * M[6]);
  f32 scaleZ = sqrtf(M[8] * M[8] + M[9] * M[9] + M[10] * M[10]);
  *scale = {scaleX, scaleY, scaleZ};

#if 0
  scaleX = fabsf(scaleX);
  scaleY = fabsf(scaleY);
  scaleZ = fabsf(scaleZ);
#endif

  f32 R[9] = {
      M[0] / scaleX, M[1] / scaleX, M[2] / scaleX,
      M[4] / scaleY, M[5] / scaleY, M[6] / scaleY,
      M[8] / scaleZ, M[9] / scaleZ, M[10] / scaleZ,
  };
  *rotation = quatFromRotationMatrix(R);
}

static inline void decompose(const f32 matrix[16],
                             Vec3 *translation,
                             Quat *rotation,
                             Vec3 *scale) {
  *translation = {matrix[12], matrix[13], matrix[14]};

  decompose(matrix, rotation, scale);
}

static inline f32 abs(f32 x) {
  return (x > 0) ? x : -x;
}

static inline f32 dist_l1(const Vec3 &lhs, const Vec3 &rhs) {
  return abs(lhs.x - rhs.x) + abs(lhs.y - rhs.y) + abs(lhs.z - rhs.z);
}

static inline f32 dist_l1(const Vec4 &lhs, const Vec4 &rhs) {
  return abs(lhs.x - rhs.x) + abs(lhs.y - rhs.y) + abs(lhs.z - rhs.z) +
         abs(lhs.w - rhs.w);
}

static inline Quat conjugate(const Quat &q) {
  return Quat(-q.x, -q.y, -q.z, q.w);
}

static inline Quat negate(const Quat &q) {
  return Quat(-q.x, -q.y, -q.z, -q.w);
}

static inline Quat operator*(const Quat &x, const Quat &y) {
  Quat ret;

  ret.x = x.x * y.w + x.w * y.x + x.y * y.z - x.z * y.y;
  ret.y = x.y * y.w + x.w * y.y + x.z * y.x - x.x * y.z;
  ret.z = x.z * y.w + x.w * y.z + x.x * y.y - x.y * y.x;
  ret.w = x.w * y.w - x.x * y.x - x.y * y.y - x.z * y.z;

  return ret;
}

}  // namespace sn::gltf
