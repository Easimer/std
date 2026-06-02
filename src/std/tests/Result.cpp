#include "std/Result.hpp"
#include "std/Testing.hpp"

struct TestError {
  i32 rc;
};

struct Value {
  f32 x;

  Value() : x(0.0f) {}
  Value(f32 x) : x(x) {}
};

static const f32 X = 5.0f;
static const i32 RC = -6;

#define CHECK_HAS_VALUE(Res, Value) \
  CHECK(Res.isOk());                \
  CHECK(!Res.isErr());              \
  CHECK(Res->x == Value);

#define CHECK_HAS_ERROR(Res, Value) \
  CHECK(!Res.isOk());               \
  CHECK(Res.isErr());               \
  CHECK(Res.unwrapErr().rc == Value);

// INIT WITH VALUE

SN_TEST(Result, ctor) {
  auto res = Result<Value, TestError>(Value(X));
  CHECK_HAS_VALUE(res, X);
}

SN_TEST(Result, moveCtor) {
  Value v(X);
  auto res = Result<Value, TestError>(std::move(v));
  CHECK_HAS_VALUE(res, X);
}

SN_TEST(Result, assign) {
  Result<Value, TestError> res = Value(X);
  CHECK_HAS_VALUE(res, X);
}

SN_TEST(Result, moveAssign) {
  Value v(X);
  Result<Value, TestError> res = std::move(v);
  CHECK_HAS_VALUE(res, X);
}

// INIT WITH ERROR

SN_TEST(Result, ctorErr) {
  auto res = Result<Value, TestError>(TestError{RC});
  CHECK_HAS_ERROR(res, RC);
}

SN_TEST(Result, moveCtorErr) {
  TestError v(RC);
  auto res = Result<Value, TestError>(std::move(v));
  CHECK_HAS_ERROR(res, RC);
}

SN_TEST(Result, assignErr) {
  Result<Value, TestError> res = TestError(RC);
  CHECK_HAS_ERROR(res, RC);
}

SN_TEST(Result, moveAssignErr) {
  TestError v(RC);
  Result<Value, TestError> res = std::move(v);
  CHECK_HAS_ERROR(res, RC);
}

SN_TEST(Result, flattenInnerValue) {
  Result<Value, TestError> inner = Value(X);

  Result<Result<Value, TestError>, TestError> outer = inner;

  Result<Value, TestError> flattened = flatten(outer);
  CHECK_HAS_VALUE(flattened, X);
}

SN_TEST(Result, flattenInnerError) {
  Result<Value, TestError> inner = TestError(RC);

  Result<Result<Value, TestError>, TestError> outer = inner;

  Result<Value, TestError> flattened = flatten(outer);
  CHECK_HAS_ERROR(flattened, RC);
}

SN_TEST(Result, flattenOuterError) {
  Result<Result<Value, TestError>, TestError> outer = TestError(RC);
  CHECK(outer.isErr());

  Result<Value, TestError> flattened = flatten(outer);
  CHECK_HAS_ERROR(flattened, RC);
}
