/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "./CompilerInfo.h"
#include "./Modules.h"
#include "./Sanitizer.h"
#include "./Types.h"

#if SN_STD_BUILDING
#define SN_STD_API SN_DLLEXPORT
#else
#define SN_STD_API SN_DLLIMPORT
#endif

typedef struct Arena Arena;

#if __cplusplus
extern "C" {
#endif
void restoreArena(Arena *dst, Arena saved);
#if __cplusplus
}
#endif

struct Arena {
  u8 *beg;
  u8 *end;

#if __cplusplus
  struct Scope;
#endif
};

typedef struct ArenaSaved {
  Arena *arena;
  Arena saved;
} ArenaSaved;

typedef ArenaSaved ArenaTemp;

#if __cplusplus
struct Arena::Scope {
  Arena *arena;
  Arena saved;

  Scope(Arena *arena) : arena(arena) { saved = *arena; }
  Scope(ArenaSaved temp) : arena(temp.arena), saved(temp.saved) {}
  ~Scope() { reset(); }
  Scope(const Scope &) = delete;
  Scope(Scope &&) = delete;
  void operator=(const Scope &) = delete;
  void operator=(Scope &&) = delete;
  operator Arena *() { return arena; }

  void reset() { restoreArena(arena, saved); }

  [[deprecated]] Scope *operator->() { return this; }
};
#endif

#if __cplusplus
extern "C" {
#endif

u8 *alloc(Arena *a, u32 objsize, u32 align, u32 count);
u8 *allocNZ(Arena *a, u32 objsize, u32 align, u32 count);
/**
 * Finds a scratch arena that doesn't conflict with the provided arenas, saves
 * its state and returns it to the caller.
 *
 * Functions that need to temporarily allocate memory on the heap can use this
 * function to acquire an arena. It's guaranteed that the returned arena is not
 * within the provided conflict list.
 *
 * When a function returns a heap allocated result and but it also needs to
 * allocate memory for some temporary results, then it would call this function
 * like this:
 *
 * ArenaSaved temp = getScratch(&arena, 1);
 *
 * where `arena` was supplied by the caller of the function as the place where
 * the result has to be allocated.
 *
 * If a function doesn't use an arena but it still has to allocate temporary
 * memory, it can call this function with an empty conflict list:
 *
 * `ArenaSaved temp = getScratch(nullptr, 0);`
 *
 * The `temp` value contains a pointer to an arena which can be used like this:
 *
 * `u8* ptr = alloc(temp.arena, 32, 1, 1);`
 *
 * Callers **must** release the ArenaSaved at the end of its scope:
 *
 * `releaseScratch(temp);`
 *
 * Callers can also reset the scratch arena to the initial state, for example at
 * the start of iterations:
 *
 * `for(...) { resetScratch(temp);  work(temp.arena); }`
 *
 */
SN_STD_API ArenaSaved getScratch(Arena **pConflicts, u32 numConflicts);
SN_STD_API void setAllocatorsForThread(Arena *arena0, Arena *arena1);
void handleOOM(Arena *arena);

/**
 * Saves the current state of the provided arena into an ArenaSaved structure.
 * The arena can be restored later to this state by `restoreArena`.
 */
#if __cplusplus
#define saveArena(pArena) \
  { (pArena), *(pArena) }
#else
#define saveArena(pArena) ((ArenaSaved){(pArena), *(pArena)})
#endif

/**
 * Restores the state of an arena using the provided value. Also notifies
 * any address sanitizers about the memory just freed.
 */
void restoreArena(Arena *dst, Arena saved);

#define releaseScratch(arenaTemp) restoreArena(arenaTemp)
#define resetScratch(arenaTemp) restoreArena(arenaTemp)

#if __cplusplus
}
#endif

#if __cplusplus

#if SN_GCC || SN_CLANG
#define SN_ASSUME_ALIGNED(expr, alignment) \
  __builtin_assume_aligned(expr, alignment)
#else
#define SN_ASSUME_ALIGNED(expr, alignment) (expr)
#endif

template <typename T>
T *alloc(Arena *a, u32 count = 1) {
  void *ptr = alloc(a, sizeof(T), alignof(T), count);
  return reinterpret_cast<T *>(SN_ASSUME_ALIGNED(ptr, alignof(T)));
}

template <typename T>
T *allocNZ(Arena *a, u32 count = 1) {
  void *ptr = allocNZ(a, sizeof(T), alignof(T), count);
  return reinterpret_cast<T *>(SN_ASSUME_ALIGNED(ptr, alignof(T)));
}

/**
 * A wrapper around ArenaSaved that automatically releases it at the end of the
 * scope. The arena can also be reset manually.
 * @deprecated
 */
struct [[deprecated]] ArenaTempScoped {
  ArenaTemp temp;

  ArenaTempScoped(ArenaTemp in) : temp(in) {}
  ArenaTempScoped(Arena *arena) { temp = saveArena(arena); }
  ~ArenaTempScoped() { restoreArena(temp.arena, temp.saved); }
  ArenaTempScoped(const ArenaTempScoped &) = delete;
  ArenaTempScoped(ArenaTempScoped &&) = delete;
  void operator=(const ArenaTempScoped &) = delete;
  void operator=(ArenaTempScoped &&) = delete;

  void reset() { restoreArena(temp.arena, temp.saved); }

  ArenaTemp *operator->() { return &temp; }
};

#endif
