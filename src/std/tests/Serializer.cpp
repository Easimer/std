#include "std/Check.h"
#include "std/Testing.hpp"

#include "std/serializer/Optional.hpp"
#include "std/serializer/SegmentArrayWriter.hpp"
#include "std/serializer/Serializer2.hpp"
#include "std/serializer/SliceReader.hpp"
#include "std/serializer/SwissTable.hpp"

struct SimpleData {
  u32 f0;
  f32 f1;

  f32 coord[3];
};

SN_TEST(Serializer, scalar) {
  Arena::Scope temp;

  // Write the serialized bytes into a SegmentArray
  SegmentArrayWriter writer(temp);

  // Begin serialization: use the writer and temp arena above, version is 0 and
  // we're writing.
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  // Serialize a single i32
  i32 x = -123;
  SN_MUST_SERIALIZE(serialize(&S, &x));

  // Extract bytes
  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  // Create a reader over the extracted bytes
  SliceReader reader(serialized);

  // Begin deserialization: use the reader and temp arena above, version is 0
  // and we're reading.
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  i32 y = 0;
  SN_MUST_SERIALIZE(serialize(&S2, &y));

  ASSERT_EQUAL(y, x);
}

[[nodiscard]] static Optional<Error> serialize(sn::Serialization *S,
                                               SimpleData *val) {
  Optional<Error> err = serialize(S, &val->f0);
  if (err) {
    return err;
  }
  err = serialize(S, &val->f1);
  if (err) {
    return err;
  }
  err = serialize(S, &val->coord);
  if (err) {
    return err;
  }

  return {};
}

SN_TEST(Serializer, simpleData) {
  Arena::Scope temp;
  SegmentArrayWriter writer(temp);

  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SimpleData sd = {
      .f0 = 239048,
      .f1 = 3.1415926f,
      .coord = {1, 2, 3},
  };
  SN_MUST_SERIALIZE(serialize(&S, &sd));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);

  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  SimpleData sd2 = {};
  SN_MUST_SERIALIZE(serialize(&S2, &sd2));

  ASSERT_EQUAL(sd2.f0, sd.f0);
  ASSERT_EQUAL(sd2.f1, sd.f1);
  ASSERT_EQUAL(sd2.coord[0], sd.coord[0]);
  ASSERT_EQUAL(sd2.coord[1], sd.coord[1]);
  ASSERT_EQUAL(sd2.coord[2], sd.coord[2]);
}

SN_TEST(Serializer, variableLengthMutSlice) {
  u32 values[3] = {1, 2, 3};
  MutSlice<u32> orig = sliceFrom(values);

  Arena::Scope temp;
  SegmentArrayWriter writer(temp);

  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &orig));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  MutSlice<u32> out;
  SN_MUST_SERIALIZE(serialize(&S2, &out));

  ASSERT_EQUAL(out.length, orig.length);
  ASSERT_EQUAL(out, orig);
}

SN_TEST(Serializer, noallocMutSlice) {
  u32 values[3] = {1, 2, 3};
  MutSlice<u32> orig = sliceFrom(values);

  Arena::Scope temp;
  SegmentArrayWriter writer(temp);

  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &orig, sn::SerializeNoallocTag{}));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  u32 values_out[4] = {0, 0, 0, 0};
  MutSlice<u32> out = sliceFrom(values_out);
  SN_MUST_SERIALIZE(serialize(&S2, &out, sn::SerializeNoallocTag{}));

  ASSERT_EQUAL(out.length, orig.length);
  ASSERT_EQUAL(out, orig);
}

SN_TEST(Serializer, noallocMutSliceNotEnoughSpace) {
  u32 values[3] = {1, 2, 3};
  MutSlice<u32> orig = sliceFrom(values);

  Arena::Scope temp;
  SegmentArrayWriter writer(temp);

  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &orig, sn::SerializeNoallocTag{}));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  u32 values_out[2] = {0, 0};
  MutSlice<u32> out = sliceFrom(values_out);
  Optional<Error> err = serialize(&S2, &out, sn::SerializeNoallocTag{});
  CHECK(err.hasValue());
  CHECK(err->type == &sn::ERROR_SERIALIZER_NOT_ENOUGH_SPACE);
}

SN_TEST(Serializer, variableLengthSlice) {
  const u32 values[3] = {1, 2, 3};
  Slice<u32> orig = sliceFrom(values);

  Arena::Scope temp;
  SegmentArrayWriter writer(temp);

  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &orig));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  Slice<u32> out;
  SN_MUST_SERIALIZE(serialize(&S2, &out));

  ASSERT_EQUAL(out.length, orig.length);
  ASSERT_EQUAL(out, orig);
}

struct SaveGameDataV0 {
  i32 position[3];
};

static Optional<Error> serialize(sn::Serialization *S, SaveGameDataV0 *v) {
  SN_TRY_SERIALIZE(serialize(S, &v->position));

  return {};
}

struct SaveGameDataV1 {
  i32 position[3];

  u32 score = 0;
};

static Optional<Error> serialize(sn::Serialization *S, SaveGameDataV1 *v) {
  SN_TRY_SERIALIZE(serialize(S, &v->position));

  if (S->version >= 1) {
    SN_TRY_SERIALIZE(serialize(S, &v->score));
  }

  return {};
}

struct SaveGameDataV2 {
  i32 position[3];

  u32 numKills = 0;
  u32 numFlorbs = 0;
  u32 numDeaths = 0;
};

static Optional<Error> serialize(sn::Serialization *S, SaveGameDataV2 *v) {
  SN_TRY_SERIALIZE(serialize(S, &v->position));

  if (S->version >= 1) {
    if (S->version >= 2) {
      SN_TRY_SERIALIZE(serialize(S, &v->numKills));
      SN_TRY_SERIALIZE(serialize(S, &v->numFlorbs));
      SN_TRY_SERIALIZE(serialize(S, &v->numDeaths));
    } else {
      // Score field in V1 was removed and replaced in V2; read and discard it
      u32 discard;
      SN_TRY_SERIALIZE(serialize(S, &discard));
    }
  }

  return {};
}

struct SaveGameDataV3 {
  i32 position[3];

  u32 numKills = 0;
  u32 numDeaths = 0;
};

static Optional<Error> serialize(sn::Serialization *S, SaveGameDataV3 *v) {
  SN_TRY_SERIALIZE(serialize(S, &v->position));

  if (S->version >= 1) {
    if (S->version >= 2) {
      SN_TRY_SERIALIZE(serialize(S, &v->numKills));
      if (S->version == 2) {
        // Florb count in V2 was removed in V3; read and discard it
        u32 discard;
        SN_TRY_SERIALIZE(serialize(S, &discard));
      }
      SN_TRY_SERIALIZE(serialize(S, &v->numDeaths));
    } else {
      // Score field in V1 was removed and replaced in V2; read and discard it
      u32 discard;
      SN_TRY_SERIALIZE(serialize(S, &discard));
    }
  }

  return {};
}

SN_TEST(Serializer, versioningFieldAdded) {
  Arena::Scope temp;

  SaveGameDataV0 save0 = {
      .position = {1, 2, 3},
  };

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &save0));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  SaveGameDataV1 save1;
  SN_MUST_SERIALIZE(serialize(&S2, &save1));

  ASSERT_EQUAL(sliceFrom(save1.position), sliceFrom(save0.position));
  ASSERT_EQUAL(save1.score, 0);
}

SN_TEST(Serializer, versioningFieldReplaced) {
  Arena::Scope temp;

  SaveGameDataV1 save1 = {
      .position = {1, 2, 3},
      .score = 12,
  };

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 1,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &save1));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 1,
      .isWriting = false,
  };

  SaveGameDataV2 save2;
  SN_MUST_SERIALIZE(serialize(&S2, &save2));

  ASSERT_EQUAL(sliceFrom(save2.position), sliceFrom(save1.position));
  ASSERT_EQUAL(save2.numKills, 0);
  ASSERT_EQUAL(save2.numDeaths, 0);
  ASSERT_EQUAL(save2.numFlorbs, 0);
}

SN_TEST(Serializer, versioningFieldRemoved) {
  Arena::Scope temp;

  SaveGameDataV2 save2 = {
      .position = {1, 2, 3},
      .numKills = 7,
      .numFlorbs = 3,
      .numDeaths = 10,
  };

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 2,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &save2));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 2,
      .isWriting = false,
  };

  SaveGameDataV3 save3;
  SN_MUST_SERIALIZE(serialize(&S2, &save3));

  ASSERT_EQUAL(sliceFrom(save3.position), sliceFrom(save2.position));
  ASSERT_EQUAL(save3.numKills, save2.numKills);
  ASSERT_EQUAL(save3.numDeaths, save2.numDeaths);
}

SN_TEST(Serializer, versioningJump) {
  Arena::Scope temp;

  SaveGameDataV0 save0 = {
      .position = {1, 2, 3},
  };

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &save0));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  SaveGameDataV3 save3;
  SN_MUST_SERIALIZE(serialize(&S2, &save3));

  ASSERT_EQUAL(sliceFrom(save3.position), sliceFrom(save0.position));
  ASSERT_EQUAL(save3.numKills, 0);
  ASSERT_EQUAL(save3.numDeaths, 0);
}

SN_TEST(Serializer, optional) {
  Optional<u32> none;
  Optional<u32> some = u32(123);

  Arena::Scope temp;

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SN_MUST_SERIALIZE(serialize(&S, &none));
  SN_MUST_SERIALIZE(serialize(&S, &some));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  Optional<u32> none_out = u32(987);
  Optional<u32> some_out;

  SN_MUST_SERIALIZE(serialize(&S2, &none_out));
  SN_MUST_SERIALIZE(serialize(&S2, &some_out));

  CHECK(!none_out.hasValue());
  CHECK(some_out.hasValue());
  ASSERT_EQUAL(some_out.value(), some.value());
}

SN_TEST(Serializer, swissTableSimple) {
  Arena::Scope temp;

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  const u32 primes[] = {2, 3, 5, 7, 11, 13, 17};

  SwissTable<u32, u32> table(temp);
  for (auto [p, i] : sliceFrom(primes)) {
    table.put(u32(i), p);
  }

  SN_MUST_SERIALIZE(serialize(&S, &table));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };
  SwissTable<u32, u32> table_out(temp);
  SN_MUST_SERIALIZE(serialize(&S2, &table_out));

  for (auto [p, i] : sliceFrom(primes)) {
    u32 *v = table_out.get(u32(i));
    CHECK(v != nullptr);
    ASSERT_EQUAL(p, *v);
  }
}

SN_TEST(Serializer, swissTableStringKey) {
  Arena::Scope temp;

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  SwissTable<char, u32> table(temp);
  table.put(sliceFromConstChar("hi"), 13);
  table.put(sliceFromConstChar("hello"), 50);
  table.put(sliceFromConstChar("asdasdasdasd"), 65535);

  SN_MUST_SERIALIZE(serialize(&S, &table));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };
  SwissTable<char, u32> table_out(temp);
  SN_MUST_SERIALIZE(serialize(&S2, &table_out));

  ASSERT_EQUAL(*table_out.get(sliceFromConstChar("hi")), 13);
  ASSERT_EQUAL(*table_out.get(sliceFromConstChar("hello")), 50);
  ASSERT_EQUAL(*table_out.get(sliceFromConstChar("asdasdasdasd")), 65535);
}

enum Option {
  OPTION_WAIT_FOR_SIGNAL = 0,
  OPTION_BLIND = 1,
  OPTION_SOLID_TO_PLAYERS = 2,
};

SN_TEST(Serializer, enumsI32) {
  Arena::Scope temp;

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  Option opt = OPTION_SOLID_TO_PLAYERS;

  SN_MUST_SERIALIZE(serialize(&S, &opt, sn::SerializeEnumTag<i32>{}));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };
  Option opt_out;
  SN_MUST_SERIALIZE(serialize(&S2, &opt_out, sn::SerializeEnumTag<i32>{}));

  ASSERT_EQUAL(opt_out, opt);
}

SN_TEST(Serializer, enumsU8) {
  Arena::Scope temp;

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  Option opt = OPTION_SOLID_TO_PLAYERS;

  SN_MUST_SERIALIZE(serialize(&S, &opt, sn::SerializeEnumTag<u8>{}));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };
  Option opt_out;
  SN_MUST_SERIALIZE(serialize(&S2, &opt_out, sn::SerializeEnumTag<u8>{}));

  ASSERT_EQUAL(opt_out, opt);
}

enum class NpcClass {
  Humans = 0,
  Aliens = 1,
  AlienFauna = 2,
  Neutral = 3,
};

SN_TEST(Serializer, enumClass) {
  Arena::Scope temp;

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  NpcClass nc = NpcClass::AlienFauna;

  SN_MUST_SERIALIZE(serialize(&S, &nc, sn::SerializeEnumTag<u8>{}));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  NpcClass nc_out;
  SN_MUST_SERIALIZE(serialize(&S2, &nc_out, sn::SerializeEnumTag<u8>{}));

  ASSERT_EQUAL(nc_out, nc);
}

SN_TEST(Serializer, vu128_u32) {
  Arena::Scope temp;

  SegmentArrayWriter writer(temp);
  sn::Serialization S = {
      .stream = &writer,
      .arena = temp,
      .version = 0,
      .isWriting = true,
  };

  u32 zero = 0;
  u32 nonzero = 65536;
  u32 last = 0xFFFFFFFF;

  SN_MUST_SERIALIZE(serialize(&S, &zero, sn::SerializeVU128Tag{}));
  SN_MUST_SERIALIZE(serialize(&S, &nonzero, sn::SerializeVU128Tag{}));
  SN_MUST_SERIALIZE(serialize(&S, &last, sn::SerializeVU128Tag{}));

  Slice<u8> serialized = copyToSlice(temp, writer.dst);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  u32 zero_out;
  u32 nonzero_out;
  u32 last_out;

  SN_MUST_SERIALIZE(serialize(&S2, &zero_out, sn::SerializeVU128Tag{}));
  SN_MUST_SERIALIZE(serialize(&S2, &nonzero_out, sn::SerializeVU128Tag{}));
  SN_MUST_SERIALIZE(serialize(&S2, &last_out, sn::SerializeVU128Tag{}));

  ASSERT_EQUAL(zero_out, zero);
  ASSERT_EQUAL(nonzero_out, nonzero);
  ASSERT_EQUAL(last_out, last);
}

SN_TEST(Serializer, vu128_u32_oob) {
  Arena::Scope temp;

  // VU128 that would decode to a u40(0)
  u8 invalid[6] = {0xF4, 0, 0, 0, 0, 0};
  Slice<u8> serialized = sliceFrom(invalid);
  CHECK(!serialized.empty());

  SliceReader reader(serialized);
  sn::Serialization S2 = {
      .stream = &reader,
      .arena = temp,
      .version = 0,
      .isWriting = false,
  };

  u32 out;

  Optional<Error> err = serialize(&S2, &out, sn::SerializeVU128Tag{});
  CHECK(err.hasValue());
  CHECK(err->type == &sn::ERROR_SERIALIZER_NOT_ENOUGH_SPACE);
}
