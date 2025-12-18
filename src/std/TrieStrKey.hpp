/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Hash.h"
#include "std/Slice.hpp"
#include "std/Types.h"

struct TrieStrKey {
  Slice<const char> key;
  // Hash of `key`; used during comparison
  u64 hash;
  // Truncated hash of `key`; used during trie traversal
  u64 hash2;

  TrieStrKey() : TrieStrKey({}, 0) {}
  TrieStrKey(Slice<char> key) : TrieStrKey(key.asConst()) {}
  TrieStrKey(Slice<const char> key)
      : TrieStrKey(key, fnv64(key.data, key.length)) {}
  TrieStrKey(Slice<const char> key, u64 hash) : TrieStrKey(key, hash, hash) {}
  TrieStrKey(Slice<const char> key, u64 hash, u64 hash2)
      : key(key), hash(hash), hash2(hash2) {}

  TrieStrKey &operator=(const TrieStrKey &other) = default;

  TrieStrKey operator<<=(u32 sh) { return {key, hash, hash << sh}; }
  operator u64() const { return hash2; }

  bool operator==(const TrieStrKey &other) const {
    return hash == other.hash && key == other.key;
  }
};
