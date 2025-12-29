/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Arena.h"
#include "std/Check.h"
#include "std/Sanitizer.h"
#include "std/Types.h"

#include <string.h>
#include <new>

#ifndef NDEBUG
static constexpr u32 POOL_NODE_SENTINEL = 0xBEEFB00F;
#endif

template <typename T>
struct Pool {
  struct Node {
#ifndef NDEBUG
    u32 sentinel;
#endif
    Node *next;
    Node *prev;

    SN_ASAN_POISANABLE_ALIGNAS T value;
  };

  struct Iterator {
    Node *cur;

    bool operator!=(const Iterator &other) const { return cur != other.cur; }
    void operator++() { cur = cur->next; }
    T &operator*() const { return cur->value; }

    using value_type = T;
    using reference = T &;
    using pointer = T *;
  };

  Arena *arena = nullptr;
  Node *head = nullptr;
  Node *freeListHead = nullptr;
  Pool() {}
  Pool(Arena *arena) : arena(arena) {}

  Iterator begin() const { return Iterator{head}; }
  Iterator end() const { return Iterator{nullptr}; }

  using iterator = Iterator;

  /**
   * \brief Preallocates space for `count` new objects.
   */
  void preallocate(u32 count) {
    Node *nodes = ::alloc<Node>(arena, count);
    Node *next = freeListHead;
    for (u32 i = 0; i < count; i++) {
#ifndef NDEBUG
      nodes[i].sentinel = POOL_NODE_SENTINEL;
#endif
      nodes[i].next = next;
      nodes[i].prev = nullptr;
      SN_ASAN_POISON(&nodes[i].value, sizeof(nodes[i].value));
      next = &nodes[i];
    }
    freeListHead = &nodes[count - 1];
  }

  /**
   * \brief Creates a new object, either by reusing a previously freed slot or
   * by allocating a new one from the arena.
   */
  T *alloc() {
    Node *node = freeListHead;
    if (node != nullptr) {
      // Pop a node off the free list
      DCHECK(node->prev == nullptr);
      freeListHead = node->next;
      node->next = nullptr;
      node->prev = nullptr;
      SN_ASAN_UNPOISON(&node->value, sizeof(node->value));
    } else {
      // Allocate a new node
      node = ::alloc<Node>(arena);
    }

    new (&node->value) T();

#ifndef NDEBUG
    node->sentinel = POOL_NODE_SENTINEL;
#endif

    // Insert into the used list
    node->next = head;

    if (head != nullptr) {
      DCHECK(head->prev == nullptr);
      head->prev = node;
    }

    head = node;
    DCHECK(node->prev == nullptr);

    return &node->value;
  }

  /**
   * \brief Frees the object.
   */
  void dealloc(T *value) {
    if (value == nullptr) {
      return;
    }

    constexpr size_t off = offsetof(Node, value);
    Node *node = (Node *)(((u8 *)value) - off);

    dealloc(node);
  }

  /**
   * \brief Frees the object slot.
   */
  void dealloc(Node *node) {
#ifndef NDEBUG
    DCHECK(node->sentinel == POOL_NODE_SENTINEL);
#endif
    Node *prev = node->prev;
    Node *next = node->next;

    // Destroy the value and poison its memory
    node->value.~T();
    SN_ASAN_POISON(&node->value, sizeof(node->value));

    // Remove the slot from the used list
    if (next) {
      next->prev = prev;
    }
    if (prev) {
      prev->next = next;
    }
    if (head == node) {
      DCHECK(prev == nullptr);
      head = next;
    }

    // Insert the slot from the freelist
    node->prev = nullptr;
    node->next = freeListHead;
    freeListHead = node;

    DCHECK(freeListHead == nullptr || freeListHead->prev == nullptr);
    DCHECK(head == nullptr || head->prev == nullptr);
  }

  /**
   * \brief Frees every object in the pool.
   */
  void clear() {
    while (head != nullptr) {
      dealloc(head);
    }
  }
};

template <typename T>
void Pool_init(Pool<T> *self, Arena *arena) {
  memset(self, 0, sizeof(Pool<T>));
  self->arena = arena;
}

template <typename T>
void Pool_preallocate(Pool<T> *self, u32 count) {
  self->preallocate(count);
}

template <typename T>
T *alloc(Pool<T> *pool) {
  DCHECK(pool);
  return pool->alloc();
}

template <typename T>
void dealloc(Pool<T> *pool, typename Pool<T>::Node *node) {
  pool->dealloc(node);
}

template <typename T>
void dealloc(Pool<T> *pool, T *value) {
  pool->dealloc(value);
}

template <typename T>
void clear(Pool<T> *self) {
  self->clear();
}
