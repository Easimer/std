/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/Check.h>
#include <std/os/OsInfo.h>
#include <std/os/Sync.h>

#if SN_STD_SYSTEM_HAS_PTHREADS
#elif SN_STD_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct Barrier {
  SYNCHRONIZATION_BARRIER inner;
};

Barrier *barrierCreate(Arena *arena, u32 count) {
  Barrier *b = (Barrier *)alloc(arena, sizeof(Barrier), __alignof(Barrier), 1);

  BOOL rc = InitializeSynchronizationBarrier(&b->inner, (LONG)count, -1);
  DCHECK(rc);

  return b;
}

void barrierSync(Barrier *barrier) {
  DWORD flags = 0;
  EnterSynchronizationBarrier(&barrier->inner, flags);
}

void barrierDestroy(Barrier *barrier) {
  BOOL rc = DeleteSynchronizationBarrier(&barrier->inner);
  DCHECK(rc);
}

#endif
