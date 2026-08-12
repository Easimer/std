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

#include <stdint.h>
#include <string.h>

/** @file Vector.hpp */

/**
 * \addtogroup Vector Vector
 * @{
 */

/**
 * A growable array.
 */
template <typename T>
struct Vector {
  T *data = nullptr;
  size_t length = 0;
  size_t capacity = 0;

  T &operator[](size_t i) const {
    DCHECK(i < length);
    return data[i];
  }

  static const size_t GROW_FACTOR_MUL = 24;
  static const size_t GROW_FACTOR_DIV = 16;
  static const size_t MAX_CAPACITY = SIZE_MAX / GROW_FACTOR_MUL;
};

/**
 * \brief Allocates spaces for `count` items in the vector and return the base
 * address to the caller. When the vector is full, a new backing array is
 * allocated into the provided arena and old elements are copied into it.
 */
template <typename T>
T *append(Arena *arena, Vector<T> *dst, size_t count) {
  if (dst->length + count > dst->capacity) {
    CHECK(dst->capacity <= Vector<T>::MAX_CAPACITY);
    size_t capRequired = dst->length + count;
    size_t newCap = dst->capacity;
    do {
      newCap =
          (newCap * Vector<T>::GROW_FACTOR_MUL) / Vector<T>::GROW_FACTOR_DIV;
      if (newCap == 0) {
        newCap = 4;
      }
    } while (newCap < capRequired);

    T *newData = alloc<T>(arena, newCap);
    if (dst->data != nullptr) {
      memcpy(newData, dst->data, dst->capacity * sizeof(T));
    }
    dst->data = newData;
    dst->capacity = newCap;
  }

  CHECK(dst->length + count <= dst->capacity);
  T *ret = &dst->data[dst->length];
  dst->length += count;
  return ret;
}

/**
 * \brief Allocates a new slot in the vector and returns it to the caller. When
 * the vector is full, a new backing array is allocated into the provided arena
 * and old elements are copied into it.
 */
template <typename T>
T *append(Arena *arena, Vector<T> *dst) {
  if (dst->length + 1 > dst->capacity) {
    CHECK(dst->capacity <= Vector<T>::MAX_CAPACITY);
    size_t newCap = (dst->capacity * Vector<T>::GROW_FACTOR_MUL) /
                    Vector<T>::GROW_FACTOR_DIV;
    if (newCap == 0) {
      newCap = 4;
    }

    T *newData = alloc<T>(arena, newCap);
    if (dst->data != nullptr) {
      memcpy(newData, dst->data, dst->capacity * sizeof(T));
    }
    dst->data = newData;
    dst->capacity = newCap;
  }

  CHECK(dst->length + 1 <= dst->capacity);
  T *ret = &dst->data[dst->length];
  dst->length++;
  return ret;
}

/**
 * \brief Appends the specified value to the vector, growing its capacity if
 * needed.
 */
template <typename T>
T *appendVal(Arena *arena, Vector<T> *dst, const T &value) {
  T *p = append(arena, dst);
  *p = value;
  return p;
}

/**
 * \brief Creates a vector with a predefined initial capacity.
 */
template <typename T>
Vector<T> vectorWithInitialCapacity(Arena *arena, size_t capacity) {
  Vector<T> ret;
  ret.data = alloc<T>(arena, capacity);
  ret.length = 0;
  ret.capacity = capacity;
  return ret;
}

/**@}*/
