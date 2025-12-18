/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Arena.h"

template <typename K, typename V>
struct Trie {
  using Ptr = Trie<K, V> *;
  Ptr children[4];
  bool deleted;
  K key;
  V value;
};

template <typename K, typename V>
V *upsert(Trie<K, V> **m, K key, Arena *arena, bool &wasPresent) {
  for (K h = key; (*m) != nullptr; h <<= 2) {
    if (key == (*m)->key) {
      bool deleted = (*m)->deleted;
      if ((*m)->deleted) {
        // pArena is not null -> caller wants to insert a new node; undelete the
        // old node
        if (arena != nullptr) {
          (*m)->deleted = false;
          memset(&(*m)->value, 0, sizeof(V));
          return &(*m)->value;
        }

        return nullptr;
      }
      wasPresent = true;
      return &(*m)->value;
    }
    m = &(*m)->children[(u64)h >> 62];
  }

  wasPresent = false;
  if (!arena) {
    return nullptr;
  }

  *m = alloc<Trie<K, V>>(arena);
  (*m)->key = key;
  (*m)->deleted = false;
  memset(&(*m)->value, 0, sizeof(V));
  return &(*m)->value;
}

template <typename K, typename V>
bool remove(Trie<K, V> **m, K key) {
  for (K h = key; (*m) != nullptr; h <<= 2) {
    if (key == (*m)->key) {
      if ((*m)->deleted) {
        return false;
      }

      (*m)->value.~V();
      (*m)->deleted = true;
      return true;
    }
    m = &(*m)->children[h >> 62];
  }

  return false;
}

template <typename K, typename V>
V *upsert(Trie<K, V> **m, K key, Arena *arena) {
  bool discard;
  return upsert(m, key, arena, discard);
}
