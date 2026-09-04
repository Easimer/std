#include <std/Check.h>
#include <std/Optional.hpp>
#include <std/Testing.hpp>

/**
 * A non-trivial type that increments a counter at constructor and decrements
 * it at destruction. Used to test Optional<T>.
 */
struct NonTrivDtor {
  static i32 counter;
  i32 value;

  explicit NonTrivDtor(i32 v) : value(v) { NonTrivDtor::counter += value; }
  ~NonTrivDtor() { NonTrivDtor::counter -= value; }

  NonTrivDtor(const NonTrivDtor &other) : value(other.value) {
    NonTrivDtor::counter += value;
  }
  NonTrivDtor(NonTrivDtor &&other) noexcept : value(other.value) {
    other.value = 0;
  }

  NonTrivDtor &operator=(NonTrivDtor &&other) noexcept {
    NonTrivDtor::counter -= value;
    value = other.value;
    other.value = 0;
    return *this;
  }

  bool operator==(const NonTrivDtor &other) const noexcept {
    return value == other.value;
  }

  /**
   * Saves the current value of NonTrivDtor::counter. At the end of the scope,
   * it assert that the counter has returned to the saved value.
   */
  struct CounterCheck {
    i32 prevValue;
    CounterCheck() : prevValue(NonTrivDtor::counter) {}
    ~CounterCheck() { CHECK(NonTrivDtor::counter == prevValue); }
    CounterCheck(const CounterCheck &) = delete;
    CounterCheck(CounterCheck &&) = delete;
    void operator=(const CounterCheck &) = delete;
    void operator=(CounterCheck &&) = delete;
  };
};
i32 NonTrivDtor::counter = 0;

SN_TEST(Optional, defaultConstructedIsEmpty) {
  Optional<f32> v;

  ASSERT_IS_FALSE(v.hasValue());
}

SN_TEST(Optional, defaultConstructedIsEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> v;

  ASSERT_IS_FALSE(v.hasValue());
}

SN_TEST(Optional, dtorIsCalled_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> v = NonTrivDtor(15);

  ASSERT_EQUAL(v->value, 15);
}

SN_TEST(Optional, valueAssign) {
  Optional<f32> v = 1.0f;

  ASSERT_IS_TRUE(v.hasValue());
  ASSERT_EQUAL(v.value(), 1.0f);
  ASSERT_EQUAL(*v, 1.0f);
}

SN_TEST(Optional, valueAssign_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> v = NonTrivDtor(2);

  ASSERT_IS_TRUE(v.hasValue());
  ASSERT_EQUAL(v.value().value, 2);
  ASSERT_EQUAL((*v).value, 2);
}

SN_TEST(Optional, copyEmptyToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty;
  Optional<NonTrivDtor> present = NonTrivDtor(2);

  present = empty;

  ASSERT_IS_FALSE(present.hasValue());
}

SN_TEST(Optional, copyPresentToEmpty) {
  Optional<u32> empty;
  Optional<u32> present = 2;

  empty = present;

  ASSERT_IS_TRUE(empty.hasValue());
  ASSERT_EQUAL(empty.value(), present.value());
}

SN_TEST(Optional, copyPresentToEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty;
  Optional<NonTrivDtor> present = NonTrivDtor(2);

  empty = present;

  ASSERT_IS_TRUE(empty.hasValue());
  ASSERT_EQUAL(empty.value(), present.value());
}

SN_TEST(Optional, copyPresentToPresent) {
  Optional<u32> a = 2;
  Optional<u32> b = 3;

  a = b;

  ASSERT_EQUAL(a.value(), b.value());
}

SN_TEST(Optional, copyPresentToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> a = NonTrivDtor(2);
  Optional<NonTrivDtor> b = NonTrivDtor(3);

  a = b;

  ASSERT_EQUAL(a.value(), b.value());
}

SN_TEST(Optional, valueOr) {
  Optional<u32> empty;
  Optional<u32> present = 3;

  ASSERT_EQUAL(empty.valueOr(4), 4);
  ASSERT_EQUAL(present.valueOr(4), 3);
}

SN_TEST(Optional, moveAssignment_emptyToEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty0;
  Optional<NonTrivDtor> empty1;

  empty0 = std::move(empty1);
  ASSERT_IS_FALSE(empty0.hasValue());
}

SN_TEST(Optional, moveAssignment_presentToEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty0;
  Optional<NonTrivDtor> present0 = NonTrivDtor(2);

  empty0 = std::move(present0);
  ASSERT_IS_TRUE(empty0.hasValue());
  ASSERT_EQUAL(empty0->value, 2);
}

SN_TEST(Optional, moveAssignment_emptyToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty0;
  Optional<NonTrivDtor> present0 = NonTrivDtor(2);

  present0 = std::move(empty0);
  ASSERT_IS_FALSE(present0.hasValue());
}

SN_TEST(Optional, moveAssignment_presentToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> present0 = NonTrivDtor(2);
  Optional<NonTrivDtor> present1 = NonTrivDtor(3);

  present0 = std::move(present1);
  ASSERT_IS_TRUE(present0.hasValue());
  ASSERT_EQUAL(present0->value, 3);
}

SN_TEST(Optional, valueOr_const) {
  const Optional<u32> empty;
  u32 actual = empty.valueOr(3);
  ARG_UNUSED(actual);
}

SN_TEST(Optional, valueOr_empty) {
  const u32 expected = 3;
  Optional<u32> empty;

  u32 actual = empty.valueOr(expected);
  ASSERT_EQUAL(actual, expected);
}

SN_TEST(Optional, valueOr_present) {
  const u32 expected = 5;
  Optional<u32> present = expected;

  u32 actual = present.valueOr(3);
  ASSERT_EQUAL(actual, expected);
}

SN_TEST(Optional, valueOrElse_const) {
  const Optional<u32> empty;
  u32 actual = empty.valueOrElse([]() { return 0; });
  ARG_UNUSED(actual);
}

SN_TEST(Optional, valueOrElse_empty_result) {
  Optional<u32> empty;

  u32 actual = empty.valueOrElse([]() { return 3; });
  ASSERT_EQUAL(actual, 3);
}

SN_TEST(Optional, valueOrElse_empty_called) {
  Optional<u32> empty;

  bool wasCalled = false;
  u32 actual = empty.valueOrElse([&wasCalled]() {
    wasCalled = true;
    return 3;
  });
  ARG_UNUSED(actual);

  ASSERT_IS_TRUE(wasCalled);
}

SN_TEST(Optional, valueOrElse_present_result) {
  const u32 expected = 5;
  Optional<u32> present = expected;

  u32 actual = present.valueOrElse([]() { return 3; });
  ASSERT_EQUAL(actual, expected);
}

SN_TEST(Optional, valueOrElse_present_notCalled) {
  const u32 expected = 5;
  Optional<u32> present = expected;

  bool wasCalled = false;
  u32 actual = present.valueOrElse([&wasCalled]() {
    wasCalled = true;
    return 3;
  });
  ARG_UNUSED(actual);

  ASSERT_IS_FALSE(wasCalled);
}

struct IThing {
  static constexpr u32 EXPECTED = 0xCAFEBEEF;
  virtual u32 func() = 0;
};

struct Thing : IThing {
  u32 token;

  Thing(u32 token) : token(token) {}
  u32 func() override { return token; }
};

SN_TEST(Optional, vtableMoveConstruct) {
  auto moveConstructed = Optional<Thing>(Thing(IThing::EXPECTED));
  ASSERT_EQUAL(moveConstructed->func(), IThing::EXPECTED);
}

SN_TEST(Optional, vtableCopyConstruct) {
  Thing t(IThing::EXPECTED);
  auto copyConstructed = Optional<Thing>(t);
  ASSERT_EQUAL(copyConstructed->func(), IThing::EXPECTED);
}

SN_TEST(Optional, vtableMoveAssign) {
  Optional<Thing> moveAssignedTo;
  Thing t(IThing::EXPECTED);
  moveAssignedTo = std::move(t);
  ASSERT_EQUAL(moveAssignedTo->func(), IThing::EXPECTED);
}

SN_TEST(Optional, vtableCopyAssign) {
  Optional<Thing> copyAssignedTo;
  Thing t(IThing::EXPECTED);
  copyAssignedTo = t;
  ASSERT_EQUAL(copyAssignedTo->func(), IThing::EXPECTED);
}
