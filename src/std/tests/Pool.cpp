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

  Vec2 *elem = pool.alloc();
  CHECK(elem != nullptr);
  CHECK(pool.head != nullptr);
}

SN_TEST(Pool, deallocSucceeds) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *elem = pool.alloc();
  CHECK(elem != nullptr);
  CHECK(pool.head != nullptr);
  CHECK(pool.freeListHead == nullptr);

  pool.dealloc(elem);
  CHECK(pool.head == nullptr);
  CHECK(pool.freeListHead != nullptr);
}

SN_TEST(Pool, allocDeallocOutOfOrder) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *e0 = pool.alloc();
  Vec2 *e1 = pool.alloc();

  pool.dealloc(e0);
  pool.dealloc(e1);

  CHECK(pool.head == nullptr);
  CHECK(pool.freeListHead != nullptr);
}

SN_TEST(Pool, allocatedElemsAreIterable) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *e0 = pool.alloc();
  Vec2 *e1 = pool.alloc();
  Vec2 *e2 = pool.alloc();
  Vec2 *e3 = pool.alloc();
  Vec2 *e4 = pool.alloc();

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

  pool.preallocate(3);

  // Op does not allocate
  CHECK(pool.head == nullptr);

  CHECK(pool.freeListHead != nullptr);
  CHECK(pool.freeListHead->next != nullptr);
  CHECK(pool.freeListHead->next->next != nullptr);
  CHECK(pool.freeListHead->next->next->next == nullptr);
}

SN_TEST(Pool, clearSucceeds) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *e0 = pool.alloc();
  Vec2 *e1 = pool.alloc();

  clear(&pool);
  CHECK(pool.head == nullptr);

  CHECK(pool.freeListHead != nullptr);
  CHECK(pool.freeListHead->next != nullptr);
  CHECK(pool.freeListHead->next->next == nullptr);
}

SN_TEST(Pool, noMemoryStomp) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *e0 = pool.alloc();
  Vec2 *e1 = pool.alloc();
  Vec2 *e2 = pool.alloc();

  *e0 = {0, 1};
  *e1 = {2, 3};
  *e2 = {4, 5};

  pool.dealloc(e2);
  pool.dealloc(e0);

  CHECK(e1->x == 2);
  CHECK(e1->y == 3);

  e0 = pool.alloc();
  *e0 = {6, 7};

  CHECK(e1->x == 2);
  CHECK(e1->y == 3);
}

SN_TEST(Pool, handsBackFreedElement) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Pool<Vec2> pool(temp);

  Vec2 *e0 = pool.alloc();
  Vec2 *e1 = pool.alloc();
  Vec2 *e2 = pool.alloc();

  pool.dealloc(e2);
  pool.dealloc(e0);

  Vec2* e3 = pool.alloc();

  // e3 has the same pointer as one of the dealloc'd elements
  CHECK(e3 == e0 || e3 == e2);
}
