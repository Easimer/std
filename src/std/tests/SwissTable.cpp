#include <std/SwissTable.hpp>
#include <std/Testing.hpp>

struct TestElem {
  u64 x;
  f32 y;
};

#if !defined(__ARM_NEON)
SN_TEST(SwissTable, hasKey) {
  CHECK(impl::hasKey(0x0123456789ABCDEF, 0x01) == 0x80);
  CHECK(impl::hasKey(0x0123456789ABCDEF, 0xEF) == 0x01);
  CHECK(impl::hasKey(0x0123456789ABCDEF, 0x45) == 0x20);

  CHECK(impl::hasKey(0x0101010102020202, 0x01) == 0xF0);
  CHECK(impl::hasKey(0x0101010102020202, 0x02) == 0x0F);
  CHECK(impl::hasKey(0x0303030303030303, 0x03) == 0xFF);
}
#endif

#if defined(__ARM_NEON)
SN_TEST(SwissTable, hasKeyNEON) {
  CHECK(impl::hasKeyNEON(0x0123456789ABCDEF, 0x01) == 0x8000000000000000ULL);
  CHECK(impl::hasKeyNEON(0x0123456789ABCDEF, 0xEF) == 0x0000000000000080ULL);
  CHECK(impl::hasKeyNEON(0x0123456789ABCDEF, 0x45) == 0x0000800000000000ULL);

  CHECK(impl::hasKeyNEON(0x0101010102020202, 0x01) == 0x8080808000000000ULL);
  CHECK(impl::hasKeyNEON(0x0101010102020202, 0x02) == 0x0000000080808080ULL);
  CHECK(impl::hasKeyNEON(0x0303030303030303, 0x03) == 0x8080808080808080ULL);
}
#endif

SN_TEST(SwissTable, empty) {
  Arena::Scope temp;

  SwissTable<char, TestElem> table(temp);
  CHECK(table._empty());
}

SN_TEST(SwissTable, putSucceeds) {
  Arena::Scope temp;

  SwissTable<char, TestElem> table(temp);

  TestElem t = {123, 4.0f};
  TestElem *slot = table.put(sliceFrom("key"), t);
  CHECK(slot != nullptr);
  CHECK(slot->x == 123);
  CHECK(slot->y == 4.0f);
}

SN_TEST(SwissTable, putGet) {
  Arena::Scope temp;

  SwissTable<char, TestElem> table(temp);

  TestElem t = {123, 4.0f};
  table.put(sliceFrom("key"), t);

  TestElem *res = table.get(sliceFrom("key"));
  CHECK(res != nullptr);
  CHECK(res->x == 123);
  CHECK(res->y == 4.0f);
}

SN_TEST(SwissTable, emptyGet) {
  Arena::Scope temp;

  SwissTable<char, TestElem> table(temp);
  TestElem *res = table.get(sliceFrom("key"));
  CHECK(res == nullptr);
  CHECK(table._empty());
}

SN_TEST(SwissTable, replacement) {
  Arena::Scope temp;

  SwissTable<char, TestElem> table(temp);

  TestElem t0 = {123, 4.0f};
  TestElem t1 = {456, 8.0f};
  TestElem *slot0 = table.put(sliceFrom("key"), t0);
  CHECK(slot0 != nullptr);
  CHECK(slot0->x == t0.x);
  CHECK(slot0->y == t0.y);

  TestElem *slot1 = table.put(sliceFrom("key"), t1);
  CHECK(slot1 != nullptr);
  CHECK(slot1->x == t1.x);
  CHECK(slot1->y == t1.y);

  CHECK(slot0 == slot1);
}

SN_TEST(SwissTable, rehash) {
  Arena::Scope temp;

  SwissTable<u8, u32> table(temp);

  for (u32 i = 0; i < 1024; i++) {
    // Insert [i] = i
    Array<u8, 4> key;
    key[0] = (i / 1) % 10;
    key[1] = (i / 10) % 10;
    key[2] = (i / 100) % 10;
    key[3] = (i / 1000) % 10;
    table.put(key.asSlice(), i);

    // Test that keys up until `i` are indeed present
    for (u32 j = 0; j <= i; j++) {
      key[0] = (j / 1) % 10;
      key[1] = (j / 10) % 10;
      key[2] = (j / 100) % 10;
      key[3] = (j / 1000) % 10;
      u32 *slot = table.get(key.asSlice());
      CHECK(slot != nullptr);
      CHECK((*slot) == j);
    }

    // Test that the rest are not
    for (u32 j = i + 1; j < 1024; j++) {
      key[0] = (j / 1) % 10;
      key[1] = (j / 10) % 10;
      key[2] = (j / 100) % 10;
      key[3] = (j / 1000) % 10;
      u32 *slot = table.get(key.asSlice());
      CHECK(slot == nullptr);
    }
  }
}

SN_TEST(SwissTable, emptyRemove) {
  Arena::Scope temp;

  SwissTable<char, u32> table(temp);

  bool wasPresent = table.remove(sliceFrom("key"));
  CHECK(!wasPresent);
}

SN_TEST(SwissTable, putRemove) {
  Arena::Scope temp;

  SwissTable<char, u32> table(temp);

  u32 *slot0 = table.put(sliceFrom("key"), 123);
  CHECK(slot0 != nullptr);
  CHECK(*slot0 == 123);

  bool wasPresent = table.remove(sliceFrom("key"));
  CHECK(wasPresent);

  u32 *slot1 = table.get(sliceFrom("key"));
  CHECK(slot1 == nullptr);
}

SN_TEST(SwissTable, valueSlotReuseAfterRemove) {
  Arena::Scope temp;

  SwissTable<u32, u32> table(temp);
  // Fill up the first group
  table.put(0, 123);
  table.put(1, 123);
  table.put(2, 123);
  table.put(3, 123);
  table.put(4, 123);
  table.put(5, 123);
  table.put(6, 123);
  table.put(7, 123);

  // Delete every slot in the first group
  table.remove(0);
  table.remove(1);
  table.remove(2);
  table.remove(3);
  table.remove(4);
  table.remove(5);
  table.remove(6);
  table.remove(7);

  // Inserting a new entry may reuse the value memory allocated to the
  // (previously deleted, but now reused) slot, but its contents should be
  // overwritten with the new value
  u32 *slot = table.put(0, 456);
  CHECK(slot != nullptr);
  CHECK(*slot == 456);
}

static constexpr u64 a =
    6364136223846793005ULL; /* see TAOCP Vol 2, 3.3.4, page 108 */
static constexpr u64 c = 9754186451795953191ULL; /* some random start value */

static u64 rand(u64 *r) {
  u64 ret = (a * (*r)) + c;
  *r = ret;
  return ret;
}

enum OpKind { OP_GET, OP_PUT, OP_REMOVE, OP_COUNT };

SN_TEST(SwissTable, random64) {
  static constexpr size_t N = 64;

  // Perform random operations on a table. There are only 64 unique keys and
  // the expected state of the table is mirrored using simple arrays.
  u64 randState = 2113148651ULL;

  for (size_t j = 0; j < 1 << 5; j++) {
    Arena::Scope temp;
    SwissTable<u64, u64> table(temp);

    // Random keys
    u64 keys[N];
    // Bit i is set if keys[i] should be in the table
    u64 present = 0;
    // The value that keys[i] should have
    u64 values[N];

    for (size_t i = 0; i < N; i++) {
      keys[i] = rand(&randState);
      values[i] = 0;
    }

    for (u32 i = 0; i < 1 << 14; i++) {
      OpKind op = OpKind(rand(&randState) % OP_COUNT);

      switch (op) {
        case OP_GET: {
          u64 idx = rand(&randState) % N;
          u64 key = keys[idx];
          bool shouldBePresent = present & (1ULL << idx);
          u64 *slot = table.get(key);
          if (shouldBePresent) {
            CHECK(slot != nullptr);
            CHECK(*slot == values[idx]);
          } else {
            CHECK(slot == nullptr);
          }
          break;
        }
        case OP_PUT: {
          u64 idx = rand(&randState) % N;
          u64 key = keys[idx];
          u64 value = rand(&randState);
          u64 *slot = table.put(key, value);
          CHECK(slot != nullptr);
          CHECK(*slot == value);

          present |= (1ULL << idx);
          values[idx] = value;
          break;
        }
        case OP_REMOVE: {
          u64 idx = rand(&randState) % N;
          u64 key = keys[idx];
          bool shouldBePresent = present & (1ULL << idx);
          bool wasPresent = table.remove(key);
          present &= ~(1ULL << idx);
          CHECK(wasPresent == shouldBePresent);
          break;
        }
        default: {
          TODO();
          break;
        }
      }
    }
  }
}

SN_TEST(SwissTable, random1024) {
  // Perform random operations on a table. There are only 1024 unique keys and
  // the expected state of the table is mirrored using simple arrays.
  u64 randState = 2113148651ULL;

  static constexpr size_t N = 1024;

  for (size_t j = 0; j < 1 << 5; j++) {
    Arena::Scope temp;
    SwissTable<u32, u32> table(temp);

    // Random keys
    u32 keys[N];
    // The value that keys[i] should have
    u32 values[N];

    for (size_t i = 0; i < N; i++) {
      keys[i] = (u32)rand(&randState);
      values[i] = 0;
    }

    for (u32 i = 0; i < 1 << 14; i++) {
      OpKind op = OpKind(rand(&randState) % OP_COUNT);

      switch (op) {
        case OP_GET: {
          u64 idx = rand(&randState) % N;
          u32 key = keys[idx];
          u32 *slot = table.get(key);
          if (slot != nullptr) {
            CHECK(*slot == values[idx]);
          }
          break;
        }
        case OP_PUT: {
          u64 idx = rand(&randState) % N;
          u32 key = keys[idx];
          u32 value = (u32)rand(&randState);
          u32 *slot = table.put(key, value);
          CHECK(slot != nullptr);
          CHECK(*slot == value);
          values[idx] = value;
          break;
        }
        case OP_REMOVE: {
          u64 idx = rand(&randState) % N;
          u32 key = keys[idx];
          table.remove(key);
          break;
        }
        default: {
          TODO();
          break;
        }
      }
    }
  }
}

SN_TEST(SwissTable, swarm64) {
  // Perform random operations on a table. Only a subset of operations is
  // enabled at any time. There are only 64 unique keys and the expected
  // state of the table is mirrored using simple arrays.
  u64 randState = 2113148651ULL;

  for (size_t j = 0; j < 1 << 10; j++) {
    Arena::Scope temp;
    SwissTable<u64, u64> table(temp);

    // Random keys
    u64 keys[64];
    // Bit i is set if keys[i] should be in the table
    u64 present = 0;
    // The value that keys[i] should have
    u64 values[64];

    for (size_t i = 0; i < 64; i++) {
      keys[i] = rand(&randState);
      values[i] = 0;
    }

    u32 enabledOps = 0xFFFFFFFF;

    for (u32 i = 0; i < 1 << 14; i++) {
      OpKind op = OpKind(rand(&randState) % OP_COUNT);
      u32 mask = u32(1) << op;
      if ((mask & enabledOps) == 0) {
        continue;
      }

      if (i != 0 && (i % 128) == 0) {
        OpKind enOp = OpKind(rand(&randState) % OP_COUNT);
        u32 enMask = u32(1) << enOp;
        if (rand(&randState) & 1) {
          enabledOps |= enMask;
        } else {
          enabledOps &= ~enMask;
        }
      }

      switch (op) {
        case OP_GET: {
          u64 idx = rand(&randState) % 64;
          u64 key = keys[idx];
          bool shouldBePresent = present & (1ULL << idx);
          u64 *slot = table.get(key);
          if (shouldBePresent) {
            CHECK(slot != nullptr);
            CHECK(*slot == values[idx]);
          } else {
            CHECK(slot == nullptr);
          }
          break;
        }
        case OP_PUT: {
          u64 idx = rand(&randState) % 64;
          u64 key = keys[idx];
          u64 value = rand(&randState);
          u64 *slot = table.put(key, value);
          CHECK(slot != nullptr);
          CHECK(*slot == value);

          present |= (1ULL << idx);
          values[idx] = value;
          break;
        }
        case OP_REMOVE: {
          u64 idx = rand(&randState) % 64;
          u64 key = keys[idx];
          bool shouldBePresent = present & (1ULL << idx);
          bool wasPresent = table.remove(key);
          present &= ~(1ULL << idx);
          CHECK(wasPresent == shouldBePresent);
          break;
        }
        default: {
          TODO();
          break;
        }
      }
    }
  }
}
