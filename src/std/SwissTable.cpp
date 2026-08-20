/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/SwissTable.hpp>

#if defined(__SSE2__) || defined(_M_X64)
#include <emmintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

namespace impl {
void splitHash(u64 hash, u64 &h1, u8 &h2) {
  h1 = (hash >> 7) & 0x01FFFFFFFFFFFFFF;
  h2 = (hash >> 0) & 0x7F;
}

u8 allocateSlot(u64 &controlWordInOut, u8 h2, u8 &prevEntry) {
  u64 controlWord = controlWordInOut;
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
  // Write back new control word
  controlWordInOut = controlWord;

  return (u8)idxSlot;
}

void markSlotDeleted(u64 &controlWord, u8 idxSlot) {
  DCHECK(idxSlot < 8);
  i32 leadingZeros = idxSlot * 8;

  const u64 mask = u64(0xFF00000000000000) >> leadingZeros;
  // Clear slot
  controlWord = controlWord & (~mask);
  // Set to deleted
  controlWord = controlWord | ((u64(CW_ENTRY_DELETED) << 56) >> (leadingZeros));
}

bool hasEmptySlot(u64 controlWord) {
  return (controlWord & CW_MASK_EMPTY) != 0;
}

bool hasUnusedSlot(const u64 &controlWordIn) {
#if defined(__SSE2__) || defined(_M_X64)
  __m128i controlWord =
      _mm_loadl_epi64(reinterpret_cast<const __m128i *>(&controlWordIn));
  __m128i entries = _mm_set1_epi8(static_cast<char>(CW_ENTRY_UNUSED));
  __m128i res = _mm_cmpeq_epi8(controlWord, entries);
  return _mm_movemask_epi8(res) != 0;
#else
  u64 controlWord = controlWordIn;
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
#endif
}

u8 hasKey(const u64 &controlWord, u8 h2) {
#if defined(__SSE2__) || defined(_M_X64)
  __m128i cw = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(&controlWord));
  __m128i h2x8 = _mm_set1_epi8(static_cast<char>(h2));
  __m128i res = _mm_cmpeq_epi8(cw, h2x8);
  int mask = _mm_movemask_epi8(res);
  mask &= 0xFF;
  return (u8)mask;
#else
  u8 ret = 0;
  for (u8 groupIdx = 0; groupIdx < 8; groupIdx++) {
    ret <<= 1;

    u8 controlWordSlot =
        impl::extractSlotFromControlWord(controlWord, groupIdx);
    if (controlWordSlot == h2) {
      ret |= 1;
    }
  }

  return ret;
#endif
}

}  // namespace impl
