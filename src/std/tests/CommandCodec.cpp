#include <std/Check.h>
#include <std/Array.hpp>
#include <std/CommandDecoder.hpp>
#include <std/CommandEncoder.hpp>
#include <std/Testing.hpp>
#include <std/Vector.hpp>

enum TestFields {
  TF_UINT = 0,
  TF_FLOAT,
  TF_STRUCT,
  TF_ARRAY_OF_STRUCT0,
  TF_ARRAY_OF_STRUCT1,

  TF_MAX
};

struct Struct {
  u32 x;
  u32 y;

  bool operator!=(const Struct &other) const {
    return x != other.x || y != other.y;
  }
};

struct TestState {
  u32 uintValue = 0;
  f32 floatValue = 0.0f;
  Struct strct = {};
  Array<Struct, 2> strctArray = {};
};

struct TestEncoder : CommandEncoder<TestFields> {
  TestState state;

  TestEncoder(Arena *arena) : CommandEncoder(arena), state({}) {}

  void end() noexcept {
    pushDirtyMask();

    pushIfFlag(state.uintValue);
    pushIfFlag(state.floatValue);
    pushIfFlag(state.strct);
    pushIfFlag(state.strctArray);
  }

  void setUint(u32 value) noexcept { set<TF_UINT>(state.uintValue, value); }

  void setFloat(f32 value) noexcept { set<TF_FLOAT>(state.floatValue, value); }

  void setStruct(const Struct &s) noexcept { set<TF_STRUCT>(state.strct, s); }

  void setStructArrayElement(u32 i, const Struct &value) noexcept {
    set<TF_ARRAY_OF_STRUCT0>(state.strctArray, i, value);
  }
};

struct TestDecoder : CommandDecoder<TestFields> {
  TestDecoder(Slice<const u8> encoded) : CommandDecoder(encoded) {}

  bool decodeNext(Slice<TestFields> &changes, TestState &outState) noexcept {
    if (!beginNextDecode()) {
      return false;
    }

    readIfFlag<TF_UINT>(state.uintValue);
    readIfFlag<TF_FLOAT>(state.floatValue);
    readIfFlag<TF_STRUCT>(state.strct);
    readIfFlag<TF_ARRAY_OF_STRUCT0>(state.strctArray);
    changes = getChangeList();
    outState = state;

    return true;
  }

  TestState state;
};

SN_TEST(CommandCodec, encode_mask_uintValue) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setUint(32);

  CHECK(enc._dirtyMask == (1 << TF_UINT));
}

SN_TEST(CommandCodec, encode_state_uintValue) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setUint(32);

  CHECK(enc.state.uintValue == 32);
}

SN_TEST(CommandCodec, encode_mask_uintAndFloatValue) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setUint(32);
  enc.setFloat(1.0f);

  CHECK(enc._dirtyMask == ((1 << TF_UINT) | (1 << TF_FLOAT)));
}

SN_TEST(CommandCodec, encode_state_struct) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setStruct({1, 2});

  CHECK(enc.state.strct.x == 1);
  CHECK(enc.state.strct.y == 2);
}

SN_TEST(CommandCodec, encode_mask_array0) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setStructArrayElement(0, {2, 3});

  CHECK(enc._dirtyMask == (1 << TF_ARRAY_OF_STRUCT0));
}

SN_TEST(CommandCodec, encode_mask_array1) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setStructArrayElement(1, {1, 4});

  CHECK(enc._dirtyMask == (1 << TF_ARRAY_OF_STRUCT1));
}

SN_TEST(CommandCodec, encode_state_array1) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setStructArrayElement(1, {6, 12});

  CHECK(enc.state.strctArray[1].x == 6);
  CHECK(enc.state.strctArray[1].y == 12);
}

SN_TEST(CommandCodec, encode_mask_zeroAfterEnd) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setUint(1);
  enc.end();

  CHECK(enc._dirtyMask == 0);
}

SN_TEST(CommandCodec, encode_end_pushesBytes) {
  Arena::Scope temp = getScratch(nullptr, 0);

  TestEncoder enc(temp);
  enc.setUint(1);
  enc.end();

  CHECK(enc._buffer.length > 0);
}

SN_TEST(CommandCodec, decode_empty) {
  Arena::Scope arena0 = getScratch(nullptr, 0);

  Slice<const u8> encoded;
  {
    Arena::Scope arena1 = getScratch(&arena0.arena, 0);
    TestEncoder enc(arena1);
    encoded = enc.extractBuffer(arena0).asConst();
  }

  TestDecoder dec(encoded);

  DCHECK(dec.isOver());
}

SN_TEST(CommandCodec, decode_simple) {
  Arena::Scope arena0 = getScratch(nullptr, 0);
  Arena::Scope arena1 = getScratch(&arena0.arena, 0);

  Slice<const u8> encoded;
  {
    TestEncoder enc(arena0);

    enc.setUint(3);
    enc.setFloat(2.0f);
    enc.end();

    encoded = enc.extractBuffer(arena1).asConst();
  }

  TestDecoder dec(encoded);

  TestState state;
  Slice<TestFields> changes;

  u32 i = 0;
  while (dec.decodeNext(changes, state)) {
    CHECK(changes[0] == TestFields::TF_UINT);
    CHECK(changes[1] == TestFields::TF_FLOAT);
    CHECK(state.uintValue == 3);
    CHECK(state.floatValue == 2.0f);
    i++;
  }

  CHECK(i == 1);
}

SN_TEST(CommandCodec, decode_simple_array) {
  Arena::Scope arena0 = getScratch(nullptr, 0);
  Arena::Scope arena1 = getScratch(&arena0.arena, 0);

  Slice<const u8> encoded;
  {
    TestEncoder enc(arena0);
    enc.setStructArrayElement(0, {1, 2});
    enc.setStructArrayElement(1, {3, 4});
    enc.end();

    encoded = enc.extractBuffer(arena1).asConst();
  }

  TestDecoder dec(encoded);

  TestState state;
  Slice<TestFields> changes;

  u32 i = 0;
  while (dec.decodeNext(changes, state)) {
    CHECK(changes[0] == TestFields::TF_ARRAY_OF_STRUCT0);
    CHECK(changes[1] == TestFields::TF_ARRAY_OF_STRUCT1);
    CHECK(state.strctArray[0].x == 1);
    CHECK(state.strctArray[0].y == 2);
    CHECK(state.strctArray[1].x == 3);
    CHECK(state.strctArray[1].y == 4);
    i++;
  }

  CHECK(i == 1);
}

SN_TEST(CommandCodec, decode_multiple_commands) {
  Arena::Scope arena0 = getScratch(nullptr, 0);
  Arena::Scope arena1 = getScratch(&arena0.arena, 0);

  const u32 NUM_COMMANDS = 16;

  Slice<const u8> encoded;
  {
    TestEncoder enc(arena0);

    for (u32 i = 0; i < NUM_COMMANDS; i++) {
      enc.setUint(i + 1);
      enc.setFloat(2.0f);
      enc.end();
    }

    encoded = enc.extractBuffer(arena1).asConst();
  }

  TestDecoder dec(encoded);

  TestState state;
  Slice<TestFields> changes;

  u32 i = 0;
  while (dec.decodeNext(changes, state)) {
    if (i == 0) {
      CHECK(changes.length == 2);
      // the float is set as well but always to 2.0f so it should only be change
      // in the first command
      CHECK(changes[0] == TestFields::TF_UINT);
      CHECK(changes[1] == TestFields::TF_FLOAT);
    } else {
      CHECK(changes.length == 1);
      CHECK(changes[0] == TestFields::TF_UINT);
    }

    CHECK(state.uintValue == i + 1);
    CHECK(state.floatValue == 2.0f);
    i++;
  }

  CHECK(i == NUM_COMMANDS);
}

SN_TEST(CommandCodec, decode_multiple_commands_array) {
  Arena::Scope arena0 = getScratch(nullptr, 0);
  Arena::Scope arena1 = getScratch(&arena0.arena, 0);

  Slice<const u8> encoded;
  {
    TestEncoder enc(arena0);

    enc.setStructArrayElement(0, {1, 2});
    enc.setStructArrayElement(1, {2, 3});
    enc.end();

    enc.setStructArrayElement(1, {4, 5});
    enc.end();

    enc.setStructArrayElement(0, {1, 1});
    enc.end();

    encoded = enc.extractBuffer(arena1).asConst();
  }

  TestDecoder dec(encoded);

  TestState state;
  Slice<TestFields> changes;

  CHECK(dec.decodeNext(changes, state));
  CHECK(changes.length == 2);
  CHECK(changes[0] == TestFields::TF_ARRAY_OF_STRUCT0);
  CHECK(changes[1] == TestFields::TF_ARRAY_OF_STRUCT1);
  CHECK(state.strctArray[0].x == 1);
  CHECK(state.strctArray[0].y == 2);
  CHECK(state.strctArray[1].x == 2);
  CHECK(state.strctArray[1].y == 3);

  CHECK(dec.decodeNext(changes, state));
  CHECK(changes.length == 1);
  CHECK(changes[0] == TestFields::TF_ARRAY_OF_STRUCT1);
  CHECK(state.strctArray[0].x == 1);
  CHECK(state.strctArray[0].y == 2);
  CHECK(state.strctArray[1].x == 4);
  CHECK(state.strctArray[1].y == 5);

  CHECK(dec.decodeNext(changes, state));
  CHECK(changes.length == 1);
  CHECK(changes[0] == TestFields::TF_ARRAY_OF_STRUCT0);
  CHECK(state.strctArray[0].x == 1);
  CHECK(state.strctArray[0].y == 1);
  CHECK(state.strctArray[1].x == 4);
  CHECK(state.strctArray[1].y == 5);

  CHECK(dec.isOver());
}

SN_TEST(CommandCodec, decode_multiple_commands_same) {
  Arena::Scope arena0 = getScratch(nullptr, 0);
  Arena::Scope arena1 = getScratch(&arena0.arena, 0);

  Slice<const u8> encoded;
  {
    TestEncoder enc(arena0);

    enc.setUint(2);
    enc.end();
    enc.end();

    encoded = enc.extractBuffer(arena1).asConst();
  }

  TestDecoder dec(encoded);

  TestState state;
  Slice<TestFields> changes;

  CHECK(dec.decodeNext(changes, state));
  CHECK(changes.length == 1);
  CHECK(changes[0] == TestFields::TF_UINT);
  CHECK(state.uintValue == 2);

  CHECK(dec.decodeNext(changes, state));
  CHECK(changes.length == 0);
  CHECK(state.uintValue == 2);

  CHECK(dec.isOver());
}
