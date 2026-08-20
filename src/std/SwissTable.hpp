/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <std/Hash.h>
#include <std/Optional.hpp>
#include <std/SegmentArray.hpp>

namespace impl {
static const u64 CW_MASK_EMPTY = 0x8080808080808080;
static const u64 CW_ENTRY_UNUSED = 0x80;
static const u64 CW_ENTRY_DELETED = 0xFE;
static const u64 CW_ENTRY_MASK = 0x80;

/**
 * \brief Splits a hash value into the upper 57 bits and the lower 7 bits.
 */
void splitHash(u64 hash, u64 &h1, u8 &h2);
/**
 * \brief Finds an empty slot in the control word, places `h2` there and returns
 * the slot index.
 * \param controlWord Reference to a control word
 * \param h2 The low 7 bits of the hash
 * \param prevEntry Reference to a location that will store the previous value
 * at the slot (either `CW_ENTRY_UNUSED` or `CW_ENTRY_DELETED`)
 * \returns The slot index (0-7)
 */
u8 allocateSlot(u64 &controlWord, u8 h2, u8 &prevEntry);
/**
 * \brief Marks a slot deleted in the control word
 */
void markSlotDeleted(u64 &controlWord, u8 idxSlot);
/**
 * \brief Determines whether the group has an empty slot (either unused slot or
 * deleted slot).
 * \param controlWord Control word of the group
 */
bool hasEmptySlot(u64 controlWord);
/**
 * \brief Determines whether the group has an unused slot.
 * \param controlWord Control word of the group
 */
bool hasUnusedSlot(u64 controlWord);

/**
 * \brief Computes the index of the home group of a key
 * \param h1 Upper 57 bits of the hash of the key
 * \param M Number of groups in the table
 */
static inline size_t homeGroup(u64 h1, size_t M) {
  return (h1 / 8) % M;
}

/**
 * \brief Extracts a 8-bit entry from a control word.
 * \param controlWord Control word
 * \param slotIdx Entry index
 */
static inline u8 extractSlotFromControlWord(u64 controlWord, u8 slotIdx) {
  DCHECK(slotIdx < 8);
  return (controlWord >> (56 - slotIdx * 8)) & 0xFF;
}
}  // namespace impl

/**
 * \brief A Swiss table for keys of Slice<K> and values of V
 * \tparam K Key base type. Actual key type is `Slice<K>`
 * \tparam V Value type
 */
template <typename K, typename V>
struct SwissTable {
  struct Slot {
    MutSlice<K> key;
    u64 hash = 0;
    size_t idxValue = 0;
  };

  /** \brief Arena used to allocate keys */
  Arena *arena;
  /** \brief Control word of each group */
  SegmentArray<u64> controlWords;
  /** \brief Slots of each group */
  SegmentArray<Array<Slot, 8>> keys;
  /** \brief Values stored in the table */
  SegmentArray<V> values;

  SwissTable() : SwissTable(nullptr) {}
  /**
   * \brief Initializes a table that allocates keys, slots control words and
   * values into the provided arena.
   */
  SwissTable(Arena *arena)
      : arena(arena), controlWords(arena), keys(arena), values(arena) {}

  /**
   * \brief Looks up a key in the table and returns a pointer to the associated
   * value. \param key Key \returns A valid pointer to the associated value if
   * the key is present in the table, or nullptr.
   */
  V *get(Slice<K> key) {
    if (_empty()) {
      return nullptr;
    }

    const size_t M = controlWords.length;
    DCHECK(M != 0);

    u64 hash = hashFNV64(key);
    u64 h1;
    u8 h2;
    impl::splitHash(hash, h1, h2);

    const size_t homeGroup = impl::homeGroup(h1, M);

    for (size_t i = 0; i < M; i++) {
      size_t group = (homeGroup + i) % M;
      u64 controlWord = controlWords[group];

      // Is the elem already in this group?
      V *v = _findWithinGroup(group, key, hash, h2);
      if (v != nullptr) {
        return v;
      }

      // Is there an unused slot in this group?
      if (impl::hasUnusedSlot(controlWord)) {
        // If so, an insertion would have used that slot, therefore the key is
        // not present
        return nullptr;
      }

      // Check the next group
    }

    return nullptr;
  }

  /**
   * \brief Sets the key to the specified value.
   * \param key Key to insert or update
   * \param value New value
   * \returns A valid pointer to the slot associated with the key
   */
  V *put(Slice<K> key, const V &value) {
    if (_empty()) {
      _growAndRehash();
    }

    const size_t M = controlWords.length;
    DCHECK(M != 0);

    u64 hash = hashFNV64(key);
    u64 h1;
    u8 h2;
    impl::splitHash(hash, h1, h2);

    const size_t homeGroup = impl::homeGroup(h1, M);

    for (size_t i = 0; i < M; i++) {
      size_t group = (homeGroup + i) % M;
      u64 controlWord = controlWords[group];

      // Is the elem already in this group?
      V *v = _findWithinGroup(group, key, hash, h2);
      if (v != nullptr) {
        *v = value;
        return v;
      }

      // Is there an empty slot in this group?
      if (impl::hasUnusedSlot(controlWord)) {
        // There is an empty slot; if the key were to be present in this table,
        // it would be in this group, but it's not.
        return _insertIntoGroup(group, key, value, hash, h2);
      }

      // Check the next group for empty slots
    }

    // Probing failed; look for an empty slot
    for (size_t i = 0; i < M; i++) {
      size_t group = (homeGroup + i) % M;
      u64 controlWord = controlWords[group];

      if (!impl::hasEmptySlot(controlWord)) {
        continue;
      }

      // Is there an empty slot in this group?
      if (!impl::hasUnusedSlot(controlWord)) {
        continue;
      }

      return _insertIntoGroup(group, key, value, hash, h2);
    }

    // Insert into a new group
    _growAndRehash();
    return put(key, value);
  }

  /**
   * \brief Removes a key from the table
   * \param key Key to remove
   * \returns A value indicating whether the key was present in the table
   */
  bool remove(Slice<K> key) {
    if (_empty()) {
      return false;
    }

    const size_t M = controlWords.length;
    DCHECK(M != 0);

    u64 hash = hashFNV64(key);
    u64 h1;
    u8 h2;
    impl::splitHash(hash, h1, h2);

    const size_t homeGroup = impl::homeGroup(h1, M);

    for (size_t i = 0; i < M; i++) {
      size_t group = (homeGroup + i) % M;
      u64 controlWord = controlWords[group];

      // Is the elem already in this group?
      u8 idxSlot;
      V *v = _findWithinGroup(group, key, hash, h2, idxSlot);
      if (v != nullptr) {
        impl::markSlotDeleted(controlWords[group], idxSlot);
        return true;
      }

      // Is there an unused slot in this group?
      if (impl::hasUnusedSlot(controlWord)) {
        // If so, an insertion would have used that slot, therefore the key is
        // not present
        return false;
      }

      // Check the next group
    }

    return false;
  }

  size_t _growAndRehash() {
    DCHECK(controlWords.length == keys.length);
    size_t idx = controlWords.length;

    size_t lenDouble = controlWords.length * 5 / 2;
    lenDouble = lenDouble != 0 ? lenDouble : 1;
    while (controlWords.length < lenDouble) {
      controlWords.push(impl::CW_MASK_EMPTY);
      keys.push();
    }

    if (idx == 0) {
      // First group, no need to rehash
      return idx;
    }

    const size_t M = controlWords.length;

    Arena::Scope temp = getScratchFor(arena);

    // Make a copy of all slots and control words
    SegmentArray<Array<Slot, 8>> keysCopy(temp);
    SegmentArray<u64> controlWordsCopy(temp);
    keysCopy.grow(this->keys.numSegments);
    controlWordsCopy.grow(this->keys.numSegments);

    const size_t N = this->controlWords.length;
    for (size_t group = 0; group < N - 1; group++) {
      controlWordsCopy[group] = this->controlWords[group];
      for (size_t j = 0; j < 8; j++) {
        keysCopy[group][j] = this->keys[group][j];
      }
    }

    // Clear slots
    for (size_t group = 0; group < N; group++) {
      this->controlWords[group] = impl::CW_MASK_EMPTY;
    }

    // Reinsert keys into the grown table
    for (size_t groupOrig = 0; groupOrig < N - 1; groupOrig++) {
      u64 controlWord = controlWordsCopy[groupOrig];
      for (u8 j = 0; j < 8; j++) {
        u8 controlWordSlot = impl::extractSlotFromControlWord(controlWord, j);
        if ((controlWordSlot & impl::CW_ENTRY_MASK) != 0) {
          continue;
        }

        DCHECK(controlWordSlot != impl::CW_ENTRY_UNUSED);
        DCHECK(controlWordSlot != impl::CW_ENTRY_DELETED);

        const Slot &slot0 = keysCopy[groupOrig][j];
        u64 h1;
        u8 h2;
        impl::splitHash(slot0.hash, h1, h2);

        const size_t homeGroup = impl::homeGroup(h1, M);

        size_t i;
        for (i = 0; i < M; i++) {
          size_t group = (homeGroup + i) % M;
          if ((this->controlWords[group] & impl::CW_MASK_EMPTY) == 0) {
            continue;
          }

          u8 entry;
          size_t idxSlot =
              impl::allocateSlot(this->controlWords[group], h2, entry);
          Slot &slot = this->keys[group][idxSlot];
          slot.key = slot0.key;
          slot.hash = slot0.hash;
          slot.idxValue = slot0.idxValue;
          break;
        }

        DCHECK(i != M);
      }
    }

    return idx;
  }

  V *_insertIntoGroup(size_t idxGroup,
                      Slice<K> key,
                      const V &value,
                      u64 hash,
                      u8 h2) {
    u8 entry;
    size_t idxSlot = impl::allocateSlot(controlWords[idxGroup], h2, entry);
    Slot &slot = keys[idxGroup][idxSlot];

    slot.hash = hash;
    if (entry == impl::CW_ENTRY_UNUSED) {
      slot.key = duplicate(arena, key);
      slot.idxValue = values.length;
      values.push(value);
    } else if (entry == impl::CW_ENTRY_DELETED) {
      if (slot.key.length <= key.length) {
        // key of deleted entry was smaller than the new key; reuse the space
        // instead of allocating a new one
        MutSlice<K> k = slot.key;
        k.length = key.length;
        k.copy(key);
      } else {
        slot.key = duplicate(arena, key);
      }

      // Re-use the current idxValue
      values[slot.idxValue] = value;
    } else {
      CHECK(0);
    }

    return &values[slot.idxValue];
  }

  V *_findWithinGroup(size_t idxGroup,
                      Slice<K> key,
                      u64 hash,
                      u64 h2,
                      u8 &groupIdxOut) {
    u64 controlWord = controlWords[idxGroup];

    for (u8 groupIdx = 0; groupIdx < 8; groupIdx++) {
      u8 controlWordSlot =
          impl::extractSlotFromControlWord(controlWord, groupIdx);

      const Slot &slot = keys[idxGroup][groupIdx];
      if (controlWordSlot == h2 && slot.hash == hash && slot.key == key) {
        groupIdxOut = groupIdx;
        return &values[slot.idxValue];
      }
    }

    return nullptr;
  }

  V *_findWithinGroup(size_t idxGroup, Slice<K> key, u64 hash, u64 h2) {
    u8 groupIdx;
    return _findWithinGroup(idxGroup, key, hash, h2, groupIdx);
  }

  bool _empty() const { return controlWords.length == 0; }
};
