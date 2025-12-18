/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/SliceUtils.hpp"

b32 compareAsString(Slice<const char> left, Slice<const char> right) {
  if (left.data == nullptr || right.data == nullptr) {
    return left.data == right.data;
  }

  if (left.length != right.length) {
    return false;
  }

  return memcmp(left.data, right.data, left.length) == 0;
}

Slice<const char> fromCStr(const char *s) {
  size_t len = strlen(s);
  return {s, u32(len)};
}

Slice<const char> fromCStrWithZero(const char* s) {
  size_t len = strlen(s);
  return {s, u32(len) + 1};
}
