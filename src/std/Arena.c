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

#if SN_GCC || SN_CLANG
#define SN_STD_LIKELY(Expr) __builtin_expect((Expr), 1)
#define SN_STD_UNLIKELY(Expr) __builtin_expect((Expr), 0)
#else
#define SN_STD_LIKELY(Expr) (Expr)
#define SN_STD_UNLIKELY(Expr) (Expr)
#endif

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
  if (SN_STD_UNLIKELY(sizAlloc == 0)) {
    return NULL;
  }

  size_t pad;
  do {
    size_t sizAvailable = a->end - a->beg;
    if (SN_STD_LIKELY(sizAvailable >= sizAlloc)) {
      // Unaligned base address that has the required amount of space
      uintptr_t baseUnaligned = (uintptr_t)a->end - sizAlloc;
      // Base address that has the required alignment
      uintptr_t baseAligned = baseUnaligned & ~(sizAlign - 1);
      // Total bytes needed after alignment
      pad = (size_t)a->end - baseAligned;
      if (SN_STD_LIKELY(sizAvailable >= pad)) {
        break;
      }
    }

    handleOOM(a);
  } while (1);

  a->end -= pad;
  DCHECK(a->beg <= a->end);

  u8 *allocStart = a->end;
  DCHECK(((uintptr_t)allocStart & (sizAlign - 1)) == 0);

  // Unpoison the allocated range if running an ASAN build
  SN_ASAN_UNPOISON(allocStart, pad);

  *sizAllocOut = pad;
  return allocStart;
}

u8 *allocNZ(Arena *a, size_t sizObj, size_t sizAlign, size_t numObjects) {
  size_t discard;
  return allocImpl(a, sizObj, sizAlign, numObjects, &discard);
}

u8 *alloc(Arena *a, size_t sizObj, size_t sizAlign, size_t numObjects) {
  size_t sizAlloc;
  u8 *allocStart = allocImpl(a, sizObj, sizAlign, numObjects, &sizAlloc);
  if (SN_STD_UNLIKELY(allocStart == NULL)) {
    return NULL;
  }

  return (u8 *)memset(allocStart, 0, sizAlloc);
}

void restoreArena(Arena *dst, Arena saved) {
  u8 *regionStart = dst->end;
  u8 *regionEnd = saved.end;
  SN_ASAN_POISON(regionStart, regionEnd - regionStart);

  *dst = saved;
}
