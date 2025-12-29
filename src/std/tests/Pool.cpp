#include <std/Check.h>
#include <std/Array.hpp>
#include <std/Pool.hpp>
#include <std/Testing.hpp>

struct Vec2 {
  alignas(16) f32 x, y;
};

SN_TEST(Pool, defaultConstructedIsEmpty) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  CHECK(pool.arena == temp);
  CHECK(pool.head == nullptr);
  CHECK(pool.freeListHead == nullptr);
}

SN_TEST(Pool, allocSucceeds) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *elem = alloc(&pool);
  CHECK(elem != nullptr);
  CHECK(pool.head != nullptr);
}

SN_TEST(Pool, deallocSucceeds) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *elem = alloc(&pool);
  CHECK(elem != nullptr);
  CHECK(pool.head != nullptr);
  CHECK(pool.freeListHead == nullptr);

  dealloc(&pool, elem);
  CHECK(pool.head == nullptr);
  CHECK(pool.freeListHead != nullptr);
}

SN_TEST(Pool, allocDeallocOutOfOrder) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *e0 = alloc(&pool);
  Vec2 *e1 = alloc(&pool);

  dealloc(&pool, e0);
  dealloc(&pool, e1);

  CHECK(pool.head == nullptr);
  CHECK(pool.freeListHead != nullptr);
}

SN_TEST(Pool, allocatedElemsAreIterable) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *e0 = alloc(&pool);
  Vec2 *e1 = alloc(&pool);
  Vec2 *e2 = alloc(&pool);
  Vec2 *e3 = alloc(&pool);
  Vec2 *e4 = alloc(&pool);

  Array<Vec2 *, 5> arrElemsNotSeen = {e2, e4, e0, e1, e3};
  Slice<Vec2 *> elemsNotSeen = arrElemsNotSeen.asSlice();

  // Iterate over the pool
  for (Vec2 &e : pool) {
    // Assert that we haven't seen the current element yet
    u32 i;
    bool notSeenYet = elemsNotSeen.any([&e](Vec2 *p) { return &e == p; }, i);
    CHECK(notSeenYet);

    // Remove it from the list of not seen elements
    elemsNotSeen[i] = elemsNotSeen[0];
    shrinkFromLeft(&elemsNotSeen);
  }

  // Assert that we've seen all allocated elements
  CHECK(elemsNotSeen.empty());
}

SN_TEST(Pool, preallocateSucceeds) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Pool_preallocate(&pool, 3);

  // Op does not allocate
  CHECK(pool.head == nullptr);

  CHECK(pool.freeListHead != nullptr);
  CHECK(pool.freeListHead->next != nullptr);
  CHECK(pool.freeListHead->next->next != nullptr);
  CHECK(pool.freeListHead->next->next->next == nullptr);
}
