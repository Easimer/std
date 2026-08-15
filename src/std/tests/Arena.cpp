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

SN_TEST(Arena, alignment) {
  Arena::Scope temp;

  struct alignas(16) A {
    int x;
  };

  struct alignas(256) B {
    int x;
  };

  uintptr_t ptrA = (uintptr_t)alloc(temp, sizeof(A), alignof(A), 3);
  CHECK((ptrA & 15) == 0);

  uintptr_t ptrB = (uintptr_t)alloc(temp, sizeof(B), alignof(B), 3);
  CHECK((ptrB & 255) == 0);
}

static constexpr u64 a =
    6364136223846793005ULL; /* see TAOCP Vol 2, 3.3.4, page 108 */
static constexpr u64 c = 9754186451795953191ULL; /* some random start value */

static u64 rand(u64 *r) {
  u64 ret = (a * (*r)) + c;
  *r = ret;
  return ret;
}

SN_TEST(Arena, swarm) {
  u64 randState = 2113148651ULL;

  enum OpKind {
    SET_COUNT,
    SET_SIZE,
    INCREASE_ALIGNMENT,
    DECREASE_ALIGNMENT,
    PUSH,
    POP,
    ALLOC,
    COUNT
  };

  u32 enabled = (1 << OpKind::COUNT) - 1;

  for (size_t idxRun = 0; idxRun < 1 << 16; idxRun++) {
    // Each run starts with clean state, except randState which is not reset
    Arena::Scope temp;

    constexpr size_t STACK_SIZ = 8;
    ArenaSaved stack[STACK_SIZ];
    size_t stackIdx = 0;

    size_t sizObj = 1;
    size_t numObj = 1;
    size_t alignObj = 1;

    for (size_t idxScenario = 0; idxScenario < 1 << 8; idxScenario++) {
      // Only a random subset of operations is enabled during a scenario
      u32 toggle = rand(&randState) % OpKind::COUNT;
      enabled ^= (1 << toggle);

      for (size_t idxBurst = 0; idxBurst < 1 << 3; idxBurst++) {
        // Perform random operations on an arena

        if (enabled == 0) {
          break;
        }

        OpKind kind;
        for (;;) {
          kind = OpKind(rand(&randState) % OpKind::COUNT);

          if ((enabled & (1 << kind)) != 0) {
            break;
          }
        }

        switch (kind) {
          case SET_COUNT: {
            size_t next = rand(&randState) % 8;
            numObj = next;
            break;
          }
          case SET_SIZE: {
            size_t next = rand(&randState) % 32;
            sizObj = next;
            break;
          }
          case INCREASE_ALIGNMENT:
            if (alignObj != 128) {
              alignObj *= 2;
            }
            break;
          case DECREASE_ALIGNMENT:
            if (alignObj != 1) {
              alignObj /= 2;
            }
            break;
          case PUSH:
            if (stackIdx != STACK_SIZ) {
              stack[stackIdx] = saveArena(temp.arena);
              stackIdx++;
            }
            break;
          case POP:
            if (stackIdx != 0) {
              restoreArena(temp.arena, stack[stackIdx - 1].saved);
              stackIdx--;
            }
            break;
          case ALLOC: {
            Arena saved = *temp.arena;
            u8 *ptr = alloc(temp, sizObj, alignObj, numObj);
            CHECK(saved.beg <= ptr);
            CHECK(ptr <= saved.end);
            size_t sizAlloc = saved.end - ptr;
            CHECK(((uintptr_t)ptr & (alignObj - 1)) == 0);
            CHECK(sizAlloc >= sizObj * numObj);
            break;
          }
          default:
            CHECK(false);
            break;
        }
      }
    }
  }
}
