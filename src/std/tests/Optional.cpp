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

  explicit NonTrivDtor(i32 v) : value(v) {
    NonTrivDtor::counter += value;
  }
  ~NonTrivDtor() {
    NonTrivDtor::counter -= value;
  }

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

  CHECK(!v.hasValue());
}

SN_TEST(Optional, defaultConstructedIsEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> v;

  CHECK(!v.hasValue());
}

SN_TEST(Optional, dtorIsCalled_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> v = NonTrivDtor(15);

  CHECK(v->value == 15);
}

SN_TEST(Optional, valueAssign) {
  Optional<f32> v = 1.0f;

  CHECK(v.hasValue());
  CHECK(v.value() == 1.0f);
  CHECK(*v == 1.0f);
}

SN_TEST(Optional, valueAssign_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> v = NonTrivDtor(2);

  CHECK(v.hasValue());
  CHECK(v.value().value == 2);
  CHECK((*v).value == 2);
}

SN_TEST(Optional, copyEmptyToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty;
  Optional<NonTrivDtor> present = NonTrivDtor(2);

  present = empty;

  CHECK(!present.hasValue());
}

SN_TEST(Optional, copyPresentToEmpty) {
  Optional<u32> empty;
  Optional<u32> present = 2;

  empty = present;

  CHECK(empty.hasValue());
  CHECK(empty.value() == present.value());
}

SN_TEST(Optional, copyPresentToEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty;
  Optional<NonTrivDtor> present = NonTrivDtor(2);

  empty = present;

  CHECK(empty.hasValue());
  CHECK(empty.value() == present.value());
}

SN_TEST(Optional, copyPresentToPresent) {
  Optional<u32> a = 2;
  Optional<u32> b = 3;

  a = b;

  CHECK(a.value() == b.value());
}

SN_TEST(Optional, copyPresentToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> a = NonTrivDtor(2);
  Optional<NonTrivDtor> b = NonTrivDtor(3);

  a = b;

  CHECK(a.value() == b.value());
}

SN_TEST(Optional, valueOr) {
  Optional<u32> empty;
  Optional<u32> present = 3;

  CHECK(empty.valueOr(4) == 4);
  CHECK(present.valueOr(4) == 3);
}

SN_TEST(Optional, moveAssignment_emptyToEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty0;
  Optional<NonTrivDtor> empty1;

  empty0 = std::move(empty1);
  CHECK(!empty0.hasValue());
}

SN_TEST(Optional, moveAssignment_presentToEmpty_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty0;
  Optional<NonTrivDtor> present0 = NonTrivDtor(2);

  empty0 = std::move(present0);
  CHECK(empty0.hasValue());
  CHECK(empty0->value == 2);
}

SN_TEST(Optional, moveAssignment_emptyToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> empty0;
  Optional<NonTrivDtor> present0 = NonTrivDtor(2);

  present0 = std::move(empty0);
  CHECK(!present0.hasValue());
}

SN_TEST(Optional, moveAssignment_presentToPresent_nonTrivialDtor) {
  NonTrivDtor::CounterCheck cc;

  Optional<NonTrivDtor> present0 = NonTrivDtor(2);
  Optional<NonTrivDtor> present1 = NonTrivDtor(3);

  present0 = std::move(present1);
  CHECK(present0.hasValue());
  CHECK(present0->value == 3);
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
  CHECK(actual == expected);
}

SN_TEST(Optional, valueOr_present) {
  const u32 expected = 5;
  Optional<u32> present = expected;

  u32 actual = present.valueOr(3);
  CHECK(actual == expected);
}

SN_TEST(Optional, valueOrElse_const) {
  const Optional<u32> empty;
  u32 actual = empty.valueOrElse([]() { return 0; });
  ARG_UNUSED(actual);
}

SN_TEST(Optional, valueOrElse_empty_result) {
  Optional<u32> empty;

  u32 actual = empty.valueOrElse([]() { return 3; });
  CHECK(actual == 3);
}

SN_TEST(Optional, valueOrElse_empty_called) {
  Optional<u32> empty;

  bool wasCalled = false;
  u32 actual = empty.valueOrElse([&wasCalled]() {
    wasCalled = true;
    return 3;
  });
  ARG_UNUSED(actual);

  CHECK(wasCalled);
}

SN_TEST(Optional, valueOrElse_present_result) {
  const u32 expected = 5;
  Optional<u32> present = expected;

  u32 actual = present.valueOrElse([]() { return 3; });
  CHECK(actual == expected);
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

  CHECK(!wasCalled);
}
