#include "std/Uuid.hpp"
#include "std/Testing.hpp"

SN_TEST(Uuid, variant) {
  Uuid x(0xf81d4fae, 0x7dec, 0x11d0, 0xa765, 0x00a0c91e6bf6);

  CHECK(x.rawVariant() == 0xA);
  CHECK(x.variant() == Uuid::VARIANT_DCE);
}

SN_TEST(Uuid, v4_bytes) {
  Uuid x(Uuid::Version4{}, 0x919108f752d1, 0x320, 0x1bacf847db4148a8);

  CHECK(x.variant() == Uuid::VARIANT_DCE);
  CHECK(x.version() == 4);

  CHECK(x[0] == 0x91);
  CHECK(x[1] == 0x91);
  CHECK(x[2] == 0x08);
  CHECK(x[3] == 0xf7);

  CHECK(x[4] == 0x52);
  CHECK(x[5] == 0xd1);

  CHECK(x[6] == 0x43);
  CHECK(x[7] == 0x20);

  CHECK(x[8] == 0x9b);
  CHECK(x[9] == 0xac);

  CHECK(x[10] == 0xf8);
  CHECK(x[11] == 0x47);
  CHECK(x[12] == 0xdb);
  CHECK(x[13] == 0x41);
  CHECK(x[14] == 0x48);
  CHECK(x[15] == 0xa8);
}

SN_TEST(Uuid, v7_bytes) {
  Uuid x(Uuid::Version7{}, 0x017F22E279B0, 0xCC3, 0x18C4DC0C0C07398F);

  CHECK(x.variant() == Uuid::VARIANT_DCE);
  CHECK(x.version() == 7);

  // 017F22E2-79B0-7CC3-98C4-DC0C0C07398F
  CHECK(x[0] == 0x01);
  CHECK(x[1] == 0x7F);
  CHECK(x[2] == 0x22);
  CHECK(x[3] == 0xE2);

  CHECK(x[4] == 0x79);
  CHECK(x[5] == 0xB0);

  CHECK(x[6] == 0x7C);
  CHECK(x[7] == 0xC3);

  CHECK(x[8] == 0x98);
  CHECK(x[9] == 0xC4);

  CHECK(x[10] == 0xDC);
  CHECK(x[11] == 0x0C);
  CHECK(x[12] == 0x0C);
  CHECK(x[13] == 0x07);
  CHECK(x[14] == 0x39);
  CHECK(x[15] == 0x8F);
}

SN_TEST(Uuid, v8_bytes) {
  Uuid x(Uuid::Version8{}, 0x2489E9AD2EE2, 0xE00, 0xEC932D5F69181C0);

  CHECK(x.variant() == Uuid::VARIANT_DCE);
  CHECK(x.version() == 8);

  CHECK(x[0] == 0x24);
  CHECK(x[1] == 0x89);
  CHECK(x[2] == 0xE9);
  CHECK(x[3] == 0xAD);

  CHECK(x[4] == 0x2E);
  CHECK(x[5] == 0xE2);

  CHECK(x[6] == 0x8E);
  CHECK(x[7] == 0x00);

  CHECK(x[8] == 0x8E);
  CHECK(x[9] == 0xC9);

  CHECK(x[10] == 0x32);
  CHECK(x[11] == 0xD5);
  CHECK(x[12] == 0xF6);
  CHECK(x[13] == 0x91);
  CHECK(x[14] == 0x81);
  CHECK(x[15] == 0xC0);
}
