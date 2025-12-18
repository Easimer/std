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
#include "std/Types.h"

#include <string.h>

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

    T value;
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
};

template <typename T>
void Pool_init(Pool<T> *self, Arena *arena) {
  memset(self, 0, sizeof(Pool<T>));
  self->arena = arena;
}

template <typename T>
void Pool_preallocate(Pool<T> *self, u32 count) {
  typename Pool<T>::Node *nodes =
      alloc<typename Pool<T>::Node>(self->arena, count);
  typename Pool<T>::Node *next = self->freeListHead;
  for (u32 i = 0; i < count; i++) {
#ifndef NDEBUG
    nodes[i].sentinel = POOL_NODE_SENTINEL;
#endif
    nodes[i].next = next;
    nodes[i].prev = nullptr;
  }
  self->freeListHead = &nodes[count - 1];
}

template <typename T>
T *alloc(Pool<T> *pool) {
  DCHECK(pool);

  typename Pool<T>::Node *node = pool->freeListHead;
  if (node != nullptr) {
    DCHECK(node->prev == nullptr);
    pool->freeListHead = node->next;
    node->next = nullptr;
    node->prev = nullptr;
  } else {
    node = alloc<typename Pool<T>::Node>(pool->arena);
  }

  new (&node->value) T();

#ifndef NDEBUG
  node->sentinel = POOL_NODE_SENTINEL;
#endif
  node->next = pool->head;

  if (pool->head) {
    DCHECK(pool->head->prev == nullptr);
    pool->head->prev = node;
  }

  pool->head = node;
  DCHECK(node->prev == nullptr);

  return &node->value;
}

template <typename T>
void dealloc(Pool<T> *pool, typename Pool<T>::Node *node) {
#ifndef NDEBUG
  DCHECK(node->sentinel == POOL_NODE_SENTINEL);
#endif
  typename Pool<T>::Node *prev = node->prev;
  typename Pool<T>::Node *next = node->next;

  node->value.~T();

  if (next) {
    next->prev = prev;
  }
  if (prev) {
    prev->next = next;
  }
  if (pool->head == node) {
    DCHECK(prev == nullptr);
    pool->head = next;
  }

  node->prev = nullptr;
  node->next = pool->freeListHead;
  pool->freeListHead = node;

  DCHECK(pool->freeListHead == nullptr || pool->freeListHead->prev == nullptr);
  DCHECK(pool->head == nullptr || pool->head->prev == nullptr);
}

template <typename T>
void dealloc(Pool<T> *pool, T *value) {
  if (value == nullptr) {
    return;
  }

  constexpr size_t off = offsetof(typename Pool<T>::Node, value);
  typename Pool<T>::Node *node =
      (typename Pool<T>::Node *)(((u8 *)value) - off);

  dealloc(pool, node);
}

template <typename T>
void clear(Pool<T> *self) {
  while (self->head != nullptr) {
    dealloc(self, self->head);
  }
}
