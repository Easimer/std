#include <std/Check.h>
#include <std/Endian.h>
#include <std/Testing.hpp>

SN_TEST(Endian, be16_load_aligned) {
  u16 buf;
  u8 *buf8 = (u8 *)&buf;

  buf8[0] = 0x12;
  buf8[1] = 0x34;
  {
    i16 actual = li16be_aligned((i16 *)&buf);
    CHECK(actual == 0x1234);
  }
  {
    u16 actual = lu16be_aligned((u16 *)&buf);
    CHECK(actual == 0x1234);
  }
}

SN_TEST(Endian, be16_load_unaligned) {
  u8 bufUnaligned0[3];
  u8 *bufUnaligned = &bufUnaligned0[1];

  bufUnaligned[0] = 0x12;
  bufUnaligned[1] = 0x34;
  {
    i16 actual = li16be(bufUnaligned);
    CHECK(actual == 0x1234);
  }
  {
    u16 actual = lu16be(bufUnaligned);
    CHECK(actual == 0x1234);
  }
}

SN_TEST(Endian, be16_store_aligned) {
  union {
    u16 val;
    u8 bytes[2];
  } x;

  si16be_aligned((i16 *)&x.val, 0x1234);
  CHECK(x.bytes[0] == 0x12);
  CHECK(x.bytes[1] == 0x34);

  su16be_aligned((u16 *)&x.val, 0x4321);
  CHECK(x.bytes[0] == 0x43);
  CHECK(x.bytes[1] == 0x21);
}

SN_TEST(Endian, be16_store_unaligned) {
  u8 bytes[3];

  si16be(&bytes[1], 0x1234);
  CHECK(bytes[1] == 0x12);
  CHECK(bytes[2] == 0x34);

  su16be(&bytes[1], 0x4321);
  CHECK(bytes[1] == 0x43);
  CHECK(bytes[2] == 0x21);
}

SN_TEST(Endian, be32_load_aligned) {
  u32 buf;
  u8 *buf8 = (u8 *)&buf;

  buf8[0] = 0x12;
  buf8[1] = 0x34;
  buf8[2] = 0x56;
  buf8[3] = 0x78;
  {
    i32 actual = li32be_aligned((i32 *)&buf);
    CHECK(actual == 0x12345678);
  }
  {
    u32 actual = lu32be_aligned((u32 *)&buf);
    CHECK(actual == 0x12345678);
  }
}

SN_TEST(Endian, be32_load_unaligned) {
  u8 bufUnaligned0[5];
  u8 *bufUnaligned = &bufUnaligned0[1];

  bufUnaligned[0] = 0x12;
  bufUnaligned[1] = 0x34;
  bufUnaligned[2] = 0x56;
  bufUnaligned[3] = 0x78;
  {
    i32 actual = li32be(bufUnaligned);
    CHECK(actual == 0x12345678);
  }
  {
    u32 actual = lu32be(bufUnaligned);
    CHECK(actual == 0x12345678);
  }
}

SN_TEST(Endian, be32_store_aligned) {
  union {
    u32 val;
    u8 bytes[4];
  } x;

  si32be_aligned((i32 *)&x.val, 0x12345678);
  CHECK(x.bytes[0] == 0x12);
  CHECK(x.bytes[1] == 0x34);
  CHECK(x.bytes[2] == 0x56);
  CHECK(x.bytes[3] == 0x78);

  su32be_aligned((u32 *)&x.val, 0x87654321);
  CHECK(x.bytes[0] == 0x87);
  CHECK(x.bytes[1] == 0x65);
  CHECK(x.bytes[2] == 0x43);
  CHECK(x.bytes[3] == 0x21);
}

SN_TEST(Endian, be32_store_unaligned) {
  u8 bytes[5];

  si32be(&bytes[1], 0x12345678);
  CHECK(bytes[1] == 0x12);
  CHECK(bytes[2] == 0x34);
  CHECK(bytes[3] == 0x56);
  CHECK(bytes[4] == 0x78);

  su32be(&bytes[1], 0x87654321);
  CHECK(bytes[1] == 0x87);
  CHECK(bytes[2] == 0x65);
  CHECK(bytes[3] == 0x43);
  CHECK(bytes[4] == 0x21);
}

SN_TEST(Endian, be64_load_aligned) {
  u64 buf;
  u8 *buf8 = (u8 *)&buf;

  buf8[0] = 0x12;
  buf8[1] = 0x34;
  buf8[2] = 0x56;
  buf8[3] = 0x78;
  buf8[4] = 0x9A;
  buf8[5] = 0xBC;
  buf8[6] = 0xDE;
  buf8[7] = 0xF0;
  {
    i64 actual = li64be_aligned((i64 *)&buf);
    CHECK(actual == 0x123456789ABCDEF0);
  }
  {
    u64 actual = lu64be_aligned((u64 *)&buf);
    CHECK(actual == 0x123456789ABCDEF0);
  }
}

SN_TEST(Endian, be64_load_unaligned) {
  u8 bufUnaligned0[9];
  u8 *bufUnaligned = &bufUnaligned0[1];

  bufUnaligned[0] = 0x12;
  bufUnaligned[1] = 0x34;
  bufUnaligned[2] = 0x56;
  bufUnaligned[3] = 0x78;
  bufUnaligned[4] = 0x9A;
  bufUnaligned[5] = 0xBC;
  bufUnaligned[6] = 0xDE;
  bufUnaligned[7] = 0xF0;
  {
    i64 actual = li64be(bufUnaligned);
    CHECK(actual == 0x123456789ABCDEF0);
  }
  {
    u64 actual = lu64be(bufUnaligned);
    CHECK(actual == 0x123456789ABCDEF0);
  }
}

SN_TEST(Endian, be64_store_aligned) {
  union {
    u64 val;
    u8 bytes[8];
  } x;

  si64be_aligned((i64 *)&x.val, 0x123456789ABCDEF0);
  CHECK(x.bytes[0] == 0x12);
  CHECK(x.bytes[1] == 0x34);
  CHECK(x.bytes[2] == 0x56);
  CHECK(x.bytes[3] == 0x78);
  CHECK(x.bytes[4] == 0x9A);
  CHECK(x.bytes[5] == 0xBC);
  CHECK(x.bytes[6] == 0xDE);
  CHECK(x.bytes[7] == 0xF0);

  su64be_aligned((u64 *)&x.val, 0x0FEDCBA987654321);
  CHECK(x.bytes[0] == 0x0F);
  CHECK(x.bytes[1] == 0xED);
  CHECK(x.bytes[2] == 0xCB);
  CHECK(x.bytes[3] == 0xA9);
  CHECK(x.bytes[4] == 0x87);
  CHECK(x.bytes[5] == 0x65);
  CHECK(x.bytes[6] == 0x43);
  CHECK(x.bytes[7] == 0x21);
}

SN_TEST(Endian, be64_store_unaligned) {
  u8 bytes[9];

  si64be(&bytes[1], 0x123456789ABCDEF0);
  CHECK(bytes[1] == 0x12);
  CHECK(bytes[2] == 0x34);
  CHECK(bytes[3] == 0x56);
  CHECK(bytes[4] == 0x78);
  CHECK(bytes[5] == 0x9A);
  CHECK(bytes[6] == 0xBC);
  CHECK(bytes[7] == 0xDE);
  CHECK(bytes[8] == 0xF0);

  su64be(&bytes[1], 0x0FEDCBA987654321);
  CHECK(bytes[1] == 0x0F);
  CHECK(bytes[2] == 0xED);
  CHECK(bytes[3] == 0xCB);
  CHECK(bytes[4] == 0xA9);
  CHECK(bytes[5] == 0x87);
  CHECK(bytes[6] == 0x65);
  CHECK(bytes[7] == 0x43);
  CHECK(bytes[8] == 0x21);
}

SN_TEST(Endian, bef32_load_aligned) {
  union {
    f32 val;
    u8 bytes[4];
  } x;

  x.bytes[0] = 0xbf;
  x.bytes[1] = 0xe1;
  x.bytes[2] = 0x00;
  x.bytes[3] = 0x00;

  f32 actual = lf32be_aligned((f32 *)&x.val);
  CHECK(actual == -1.7578125f);
}

SN_TEST(Endian, bef32_load_unaligned) {
  u8 bytes[5];

  bytes[1] = 0xbf;
  bytes[2] = 0xe1;
  bytes[3] = 0x00;
  bytes[4] = 0x00;

  f32 actual = lf32be(&bytes[1]);
  CHECK(actual == -1.7578125f);
}

SN_TEST(Endian, bef32_store_aligned) {
  union {
    f32 val;
    u8 bytes[4];
  } x;

  sf32be_aligned(&x.val, -1.7578125f);

  CHECK(x.bytes[0] == 0xBF);
  CHECK(x.bytes[1] == 0xE1);
  CHECK(x.bytes[2] == 0x00);
  CHECK(x.bytes[3] == 0x00);
}

SN_TEST(Endian, bef32_store_unaligned) {
  u8 bytes[5];

  sf32be(&bytes[1], -1.7578125f);

  CHECK(bytes[1] == 0xBF);
  CHECK(bytes[2] == 0xE1);
  CHECK(bytes[3] == 0x00);
  CHECK(bytes[4] == 0x00);
}
