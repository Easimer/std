#include <std/Check.h>
#include <std/Optional.hpp>
#include <std/Testing.hpp>

SN_TEST(Optional, defaultConstructedIsEmpty) {
  Optional<f32> v;

  CHECK(!v.hasValue());
}

SN_TEST(Optional, valueAssign) {
  Optional<f32> v = 1.0f;

  CHECK(v.hasValue());
  CHECK(v.value() == 1.0f);
  CHECK(*v == 1.0f);
}

SN_TEST(Optional, copyEmptyToPresent) {
  Optional<u32> empty;
  Optional<u32> present = 2;

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

SN_TEST(Optional, copyPresentToPresent) {
  Optional<u32> a = 2;
  Optional<u32> b = 3;

  a = b;

  CHECK(a.value() == b.value());
}

SN_TEST(Optional, valueOr) {
  Optional<u32> empty;
  Optional<u32> present = 3;

  CHECK(empty.valueOr(4) == 4);
  CHECK(present.valueOr(4) == 3);
}
