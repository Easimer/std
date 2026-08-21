#include <std/Check.h>
#include <std/Testing.hpp>

#include <std/math/f32x4.hpp>
#include <std/math/i32x4.hpp>

#define ENABLE_SCALAR_TESTS

#if defined(__SSE4_2__) || defined(_M_X64)
#define ENABLE_SSE42_TESTS
#endif

#if defined(__ARM_NEON)
#define ENABLE_NEON_TESTS
#endif

#if defined(ENABLE_SCALAR_TESTS)
#include <std/math/f32x4_scalar.hpp>
#include <std/math/i32x4_scalar.hpp>

#define DECL_SCALAR_TEST(Type, Name, Func) \
  SN_TEST(Type##_scalar, Name) { Func<scalar::Type>(); }
#else
#define DECL_SCALAR_TEST(Type, Name, Func)
#endif /* defined(ENABLE_SCALAR_TESTS) */

#if defined(ENABLE_SSE42_TESTS)
#include <std/math/f32x4_sse42.hpp>
#include <std/math/i32x4_sse42.hpp>

#define DECL_SSE42_TEST(Type, Name, Func) \
  SN_TEST(Type##_sse42, Name) { Func<sse42::Type>(); }
#else
#define DECL_SSE42_TEST(Type, Name, Func)
#endif /* defined(ENABLE_SSE42_TESTS) */

#if defined(ENABLE_NEON_TESTS)
#include <std/math/f32x4_neon.hpp>
#include <std/math/i32x4_neon.hpp>

#define DECL_NEON_TEST(Type, Name, Func) \
  SN_TEST(Type##_neon, Name) { Func<neon::Type>(); }
#else
#define DECL_NEON_TEST(Type, Name, Func)
#endif /* defined(ENABLE_NEON_TESTS) */

#define DECL_TEST_FOR_ALL_ISA(Type, Name, Func) \
  DECL_SCALAR_TEST(Type, Name, Func)            \
  DECL_SSE42_TEST(Type, Name, Func)             \
  DECL_NEON_TEST(Type, Name, Func)

template <typename f32x4>
void test_init() {
  f32 buf[4];

  f32x4 zero;
  zero.storeTo(buf);
  CHECK(buf[0] == 0.0f);
  CHECK(buf[1] == 0.0f);
  CHECK(buf[2] == 0.0f);
  CHECK(buf[3] == 0.0f);

  f32x4 broadcast(4.0f);
  broadcast.storeTo(buf);
  CHECK(buf[0] == 4.0f);
  CHECK(buf[1] == 4.0f);
  CHECK(buf[2] == 4.0f);
  CHECK(buf[3] == 4.0f);

  f32x4 elems(1.0f, 2.0f, -4.0f, 8.0f);
  elems.storeTo(buf);
  CHECK(buf[0] == 1.0f);
  CHECK(buf[1] == 2.0f);
  CHECK(buf[2] == -4.0f);
  CHECK(buf[3] == 8.0f);
}

DECL_TEST_FOR_ALL_ISA(f32x4, init, test_init)

template <typename f32x4>
void test_add() {
  f32x4 lhs(1.0f, 2.0f, -4.0f, 8.0f);
  f32x4 rhs(2.0f, -4.0f, 8.0f, 8.0f);

  f32 buf[4];
  (lhs + rhs).storeTo(buf);
  CHECK(buf[0] == 3.0f);
  CHECK(buf[1] == -2.0f);
  CHECK(buf[2] == 4.0f);
  CHECK(buf[3] == 16.0f);
}

DECL_TEST_FOR_ALL_ISA(f32x4, add, test_add)

template <typename f32x4>
void test_mul() {
  f32x4 lhs(1.0f, 2.0f, -4.0f, 8.0f);
  f32x4 rhs(2.0f, -4.0f, 8.0f, 8.0f);

  f32 buf[4];
  (lhs * rhs).storeTo(buf);
  CHECK(buf[0] == 2.0f);
  CHECK(buf[1] == -8.0f);
  CHECK(buf[2] == -32.0f);
  CHECK(buf[3] == 64.0f);
}

DECL_TEST_FOR_ALL_ISA(f32x4, mul, test_mul)

template <typename f32x4>
void test_lt() {
  f32x4 lhs(1.0f, 2.0f, -4.0f, 8.0f);
  f32x4 rhs(2.0f, -4.0f, 8.0f, 8.0f);

  i32 buf[4];
  auto res = lhs < rhs;
  res.storeTo(buf);
  u32 mask = res.moveMask();

  CHECK(buf[0] != 0);
  CHECK(buf[1] == 0);
  CHECK(buf[2] != 0);
  CHECK(buf[3] == 0);

  CHECK(mask == 0b0101);
}

DECL_TEST_FOR_ALL_ISA(f32x4, lt, test_lt)

template <typename f32x4>
void test_blend() {
  f32x4 lhs(1.0f, 2.0f, -4.0f, 8.0f);
  f32x4 rhs(2.0f, -4.0f, 8.0f, 16.0f);

  f32x4 res = blend(lhs, rhs, {-1, 0, -1, 0});
  f32 buf[4];
  res.storeTo(buf);

  CHECK(buf[0] == 1.0f);
  CHECK(buf[1] == -4.0f);
  CHECK(buf[2] == -4.0f);
  CHECK(buf[3] == 16.0f);
}

DECL_TEST_FOR_ALL_ISA(f32x4, blend, test_blend)
