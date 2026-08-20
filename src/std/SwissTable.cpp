/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/SwissTable.hpp>

namespace impl {
void splitHash(u64 hash, u64 &h1, u8 &h2) {
  h1 = (hash >> 7) & 0x01FFFFFFFFFFFFFF;
  h2 = (hash >> 0) & 0x7F;
}

u8 allocateSlot(u64 &controlWord, u8 h2, u8 &prevEntry) {
  i32 leadingZeros = countLeadingZeros64(controlWord & CW_MASK_EMPTY);
  DCHECK(leadingZeros != 64);
  i32 idxSlot = leadingZeros / 8;

  const u64 mask = u64(0xFF00000000000000) >> leadingZeros;
  // Get current marker
  prevEntry = u8((controlWord & mask) >> (56 - leadingZeros));
  // Clear empty marker
  controlWord = controlWord & (~mask);
  // Insert element metadata
  controlWord = controlWord | ((u64(h2) << 56) >> (leadingZeros));

  return (u8)idxSlot;
}

void markSlotDeleted(u64 &controlWord, u8 idxSlot) {
  DCHECK(idxSlot < 8);
  i32 leadingZeros = idxSlot * 8;

  const u64 mask = u64(0xFF00000000000000) >> leadingZeros;
  // Clear slot
  controlWord = controlWord & (~mask);
  //Set to deleted 
  controlWord = controlWord | ((u64(CW_ENTRY_DELETED) << 56) >> (leadingZeros));
}

bool hasEmptySlot(u64 controlWord) {
  return (controlWord & CW_MASK_EMPTY) != 0;
}

bool hasUnusedSlot(u64 controlWord) {
  // Unused/deleted slots have their MSB set. Masking off everything except the
  // MSBs give you a bit-string where set bits signal the start of an empty
  // slot's entry.
  u64 emptySlots = controlWord & CW_MASK_EMPTY;

  while (emptySlots != 0) {
    i32 shift = countLeadingZeros64(emptySlots);
    emptySlots <<= shift;
    controlWord <<= shift;

    u64 entry = controlWord & u64(0xFF00000000000000);
    if (entry == (CW_ENTRY_UNUSED << 56)) {
      return true;
    }

    emptySlots &= ~u64(0xFF00000000000000);
  }

  return false;
}

}  // namespace impl