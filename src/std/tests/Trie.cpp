#include <std/Array.hpp>
#include <std/SliceUtils.hpp>
#include <std/Testing.hpp>
#include <std/Trie.hpp>
#include <std/TrieStrKey.hpp>

SN_TEST(TrieStrKey, lookupEmpty) {
  Arena::Scope temp;

  Trie<TrieStrKey, u32> *kv = nullptr;
  Slice<char> key = sliceFrom("test");

  bool wasPresent = false;
  u32 *slot = upsert(&kv, {key}, nullptr, wasPresent);
  CHECK(slot == nullptr);
  CHECK(!wasPresent);
}

SN_TEST(TrieStrKey, insertOneLookup) {
  Arena::Scope temp;

  Trie<TrieStrKey, u32> *kv = nullptr;
  Slice<char> key = sliceFrom("test");

  {
    bool wasPresent = false;
    u32 *slot = upsert(&kv, {key}, temp, wasPresent);
    CHECK(!wasPresent);
    CHECK(slot != nullptr);

    *slot = 8192;
  }
  {
    bool wasPresent = false;
    u32 *slot = upsert(&kv, {key}, temp, wasPresent);
    CHECK(wasPresent);
    CHECK(slot != nullptr);
    CHECK(*slot == 8192);
  }
}

SN_TEST(TrieStrKey, handlesCollisions) {
  Arena::Scope temp;

  Trie<TrieStrKey, u32> *kv = nullptr;

  Slice<char> k0 = sliceFrom("8yn0iYCKYHlIj4-BwPqk");
  Slice<char> k1 = sliceFrom("GReLUrM4wMqfg9yzV3KQ");

  u64 h0 = fnv64(k0.data, k0.byteLength());
  u64 h1 = fnv64(k1.data, k1.byteLength());

  CHECK(h0 == h1);

  u32 *slot0 = upsert(&kv, {k0}, temp);
  u32 *slot1 = upsert(&kv, {k1}, temp);

  CHECK(slot0 != slot1);
}

SN_TEST(TrieStrKey, insertManyLookup) {
  Arena::Scope temp;

  Trie<TrieStrKey, u32> *kv = nullptr;

  const u32 N = 1024;

  for (u32 i = 0; i < N; i++) {
    MutSlice<char> key;
    alloc(temp, 4, key);
    key[0] = (i / 1) % 10;
    key[1] = (i / 10) % 10;
    key[2] = (i / 100) % 10;
    key[3] = (i / 1000) % 10;

    bool wasPresent = false;
    u32 *slot = upsert(&kv, {key.asSlice()}, temp, wasPresent);
    CHECK(slot != nullptr);
    CHECK(!wasPresent);

    *slot = i;
  }

  for (u32 i = 0; i < N; i++) {
    Array<char, 4> key;
    key[0] = (i / 1) % 10;
    key[1] = (i / 10) % 10;
    key[2] = (i / 100) % 10;
    key[3] = (i / 1000) % 10;

    bool wasPresent = false;
    u32 *slot = upsert(&kv, {key.asSlice().asSlice()}, temp, wasPresent);
    CHECK(slot != nullptr);
    CHECK(wasPresent);
    CHECK(*slot == i);
  }
}