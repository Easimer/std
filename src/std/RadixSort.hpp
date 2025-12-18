/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Check.h"
#include "std/Types.h"

#include <string.h>

template <u32 Shift, typename K>
inline void countingSort(u32 N,
                         const u32 *indices,
                         u32 *indicesOut,
                         const K *keys) {
  u32 counts[256] = {};

  for (u32 i = 0; i < N; i++) {
    u32 index = indices[i];
    DCHECK(index < N);
    u8 key = (keys[index] >> Shift) & 0xFF;
    counts[key] += 1;
  }

  u32 total = 0;
  u32 numNonZero = 0;
  for (u32 i = 0; i < 256; i++) {
    u32 origCount = counts[i];
    counts[i] = total;
    total += origCount;
    if (origCount != 0) {
      numNonZero++;
    }
  }

  if (numNonZero <= 1) {
    memcpy(indicesOut, indices, N * sizeof(u32));
    return;
  }

  for (u32 i = 0; i < N; i++) {
    u32 index = indices[i];
    DCHECK(index < N);
    u8 key = (keys[index] >> Shift) & 0xFF;
    u32 count = counts[key]++;
    DCHECK(count < N);
    indicesOut[count] = indices[i];
  }
}

inline void radixSort(u32 N, u32 *indices, u32 *temp, const u32 *keys) {
  countingSort<0>(N, indices, temp, keys);
  countingSort<8>(N, temp, indices, keys);
  countingSort<16>(N, indices, temp, keys);
  countingSort<24>(N, temp, indices, keys);
}

inline void radixSort(u32 N, u32 *indices, u32 *temp, const i32 *keys) {
  radixSort(N, indices, temp, reinterpret_cast<const u32 *>(keys));
}

inline void radixSort(u32 N, u32 *indices, u32 *temp, const u64 *keys) {
  countingSort<0>(N, indices, temp, keys);
  countingSort<8>(N, temp, indices, keys);
  countingSort<16>(N, indices, temp, keys);
  countingSort<24>(N, temp, indices, keys);
  countingSort<32>(N, indices, temp, keys);
  countingSort<40>(N, temp, indices, keys);
  countingSort<48>(N, indices, temp, keys);
  countingSort<56>(N, temp, indices, keys);
}
