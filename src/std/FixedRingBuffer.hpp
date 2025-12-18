/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Check.h"
#include "std/Types.h"

#include <utility>

template <typename T, u32 Size>
struct FixedRingBuffer {
  u32 read = 0;
  u32 write = 0;

  T elems[Size];

  b32 tryPush(const T &elem) {
    if (full()) {
      return false;
    }

    elems[(write++) % Size] = elem;
    return true;
  }

  void push(T &&elem) {
    CHECK(!full());

    elems[(write++) % Size] = std::move(elem);
  }

  T pop() {
    CHECK(write > read);
    return elems[(read++) % Size];
  }

  T &peek(u32 idxElem = 0) {
    CHECK(write > read + idxElem);

    return elems[(read + idxElem) % Size];
  }

  b32 empty() const { return write == read; }
  b32 full() const { return write == read + Size; }
  u32 size() const { return write - read; }
};
