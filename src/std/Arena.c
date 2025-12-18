/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Arena.h"
#include "std/Check.h"
#include "std/Types.h"

#include <string.h>

static u8 *allocImpl(Arena *a,
                     u32 sizObj,
                     u32 sizAlign,
                     u32 numObjects,
                     size_t *sizAllocOut) {
  DCHECK(numObjects >= 0);

#ifdef SN_ASAN_ACTIVE
  // ASAN requires allocations to be 8-bytes aligned
  sizAlign = (sizAlign + 7) & (~7);
#endif

  u32 pad = (u64)a->end & (sizAlign - 1);
  while (!(numObjects < (a->end - a->beg - pad) / sizObj)) {
    handleOOM(a);
  }

  size_t sizAlloc = (size_t)sizObj * numObjects;

  u8 *allocEnd = a->end;
  a->end -= sizAlloc + pad;
  u8 *allocStart = a->end;

  // Unpoison the allocated range if running an ASAN build
#ifdef SN_ASAN_ACTIVE
  SN_ASAN_UNPOISON(allocStart, sizAlloc);
#else
  (void)allocEnd;
#endif

  *sizAllocOut = sizAlloc;
  return allocStart;
}

u8 *allocNZ(Arena *a, u32 sizObj, u32 sizAlign, u32 numObjects) {
  size_t discard;
  return allocImpl(a, sizObj, sizAlign, numObjects, &discard);
}

u8 *alloc(Arena *a, u32 sizObj, u32 sizAlign, u32 numObjects) {
  size_t sizAlloc;
  u8 *allocStart = allocImpl(a, sizObj, sizAlign, numObjects, &sizAlloc);
  return (u8 *)memset(allocStart, 0, sizAlloc);
}

void restoreArena(Arena *dst, Arena saved) {
  u8 *regionStart = dst->end;
  u8 *regionEnd = saved.end;
  SN_ASAN_POISON(regionStart, regionEnd - regionStart);

  *dst = saved;
}
