#include <std/Arena.h>
#include <std/Check.h>
#include <std/Testing.hpp>

SN_TEST(getScratch, returnsValidArena) {
  Arena *ret = getScratch(nullptr, 0).arena;
  CHECK(ret != nullptr);
}

SN_TEST(getScratch, respectsConflictArenaList) {
  Arena *arena0 = getScratch(nullptr, 0).arena;
  Arena *arena1 = getScratch(&arena0, 1).arena;
  CHECK(arena0 != nullptr);
  CHECK(arena1 != nullptr);
  CHECK(arena0 != arena1);
}

SN_TEST(ArenaScope, cleansUp) {
  auto inner = [](Arena *arena) {
    Arena::Scope temp = arena;

    u8 *bytes = alloc<u8>(temp, 128);
    (void)bytes;
  };
  ArenaSaved s = getScratch(nullptr, 0);
  inner(s.arena);

  CHECK(s.arena->beg == s.saved.beg);
  CHECK(s.arena->end == s.saved.end);
}

SN_TEST(Arena, succeeds) {
  Arena::Scope temp;

  u8 *bytes = alloc<u8>(temp, 128);
  CHECK(bytes != nullptr);
}

SN_TEST(Arena, memoryIsZeroInited) {
  Arena::Scope temp;

  u64 *values = alloc<u64>(temp, 8);
  for (u32 i = 0; i < 8; i++) {
    CHECK(values[i] == 0);
  }
}

SN_TEST_MUST_FAIL(Arena, callsHandleOomWhenOutOfSpace) {
  Arena::Scope temp;

  alloc<u8>(temp, 0xFFFFFFFF);
}

SN_TEST(Arena, exactSizeAllocSucceeds) {
  Arena::Scope temp;

  ptrdiff_t size = (temp.arena->end - temp.arena->beg);
  CHECK(size <= 0xFFFFFFFF);

  u8 *bytes = alloc<u8>(temp, (u32)size);
  CHECK(bytes != nullptr);
}

SN_TEST(ArenaScope, defaultCtor) {
  ArenaSaved saved = getScratch(nullptr, 0);

  {
    Arena::Scope temp;
    CHECK(temp.arena == saved.arena);
    alloc<u8>(temp, 64);
  }

  ArenaSaved now = getScratch(nullptr, 0);
  CHECK(now.arena->beg == saved.arena->beg);
  CHECK(now.arena->end == saved.arena->end);
}

SN_TEST(Arena, getScratchFor) {
  ArenaSaved a0 = getScratch(nullptr, 0);
  ArenaSaved a1 = getScratchFor(a0.arena);
  ArenaSaved a2 = getScratchFor(a1.arena);

  CHECK(a0.arena != nullptr);
  CHECK(a1.arena != nullptr);
  CHECK(a2.arena != nullptr);

  CHECK(a0.arena != a1.arena);
  CHECK(a1.arena != a2.arena);
  CHECK(a0.arena == a2.arena);
}
