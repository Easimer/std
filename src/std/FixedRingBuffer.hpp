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
    CHECK(read != write);
    return elems[(read++) % Size];
  }

  T &peek(u32 offElem = 0) {
    u32 idxElem = read + offElem;
    CHECK(isValidPhyIndex(idxElem));

    return elems[idxElem % Size];
  }

  bool isValidPhyIndex(u32 idxElem) {
    if (empty()) {
      return false;
    }

    if (read < write) {
      // If `read` is before `write`, then idxElem must be before the `write`:
      // |      R    E     W    |
      return idxElem < write;
    } else {
      // If `write` is before `read`, then idxElem must either
      // be before or on the last possible index:
      // |   W          R     E |
      //
      // or it must be before the write index:
      // | E W          R       |
      return (idxElem <= 0xFFFFFFFF || idxElem < write);
    }
  }

  b32 empty() const { return write == read; }
  b32 full() const { return write == read + Size; }
  u32 size() const {
    if (read <= write) {
      return write - read;
    } else {
      DCHECK(read != 0);  // not possible; this branch runs when `write < read`
                          // and there is no `write` value that would satisfy
                          // the inequality when `read` is 0
      return (0xFFFFFFFF - read + 1) + write;
    }
  }
};
