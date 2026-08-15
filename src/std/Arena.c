/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Arena.h"
#include "std/Check.h"
#include "std/Sanitizer.h"
#include "std/Types.h"

#include <stdint.h>
#include <string.h>

static u8 *allocImpl(Arena *a,
                     size_t sizObj,
                     size_t sizAlign,
                     size_t numObjects,
                     size_t *sizAllocOut) {
#ifdef SN_ASAN_ACTIVE
  // ASAN requires allocations to be 8-bytes aligned
  sizAlign = (sizAlign + 7) & (~7);
#endif

  const size_t sizAlloc = sizObj * numObjects;
  ptrdiff_t pad;
  do {
    uintptr_t baseUnaligned =
        (uintptr_t)a->end - sizAlloc;  // Unaligned base address that has the
                                       // required amount of space
    uintptr_t baseAligned =
        baseUnaligned &
        ~(sizAlign - 1);  // Base address that has the required alignment
    pad = (uintptr_t)a->end - baseAligned;  // Total bytes needed
    if (a->end - a->beg >= pad) {
      break;
    }

    handleOOM(a);
  } while (1);

  a->end -= pad;
  DCHECK(a->beg <= a->end);

  u8 *allocStart = a->end;
  DCHECK(((uintptr_t)allocStart & (sizAlign - 1)) == 0);

  // Unpoison the allocated range if running an ASAN build
  SN_ASAN_UNPOISON(allocStart, sizAlloc);

  *sizAllocOut = sizAlloc;
  return allocStart;
}

u8 *allocNZ(Arena *a, size_t sizObj, size_t sizAlign, size_t numObjects) {
  size_t discard;
  return allocImpl(a, sizObj, sizAlign, numObjects, &discard);
}

u8 *alloc(Arena *a, size_t sizObj, size_t sizAlign, size_t numObjects) {
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
