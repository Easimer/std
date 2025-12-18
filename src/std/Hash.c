/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Hash.h"

u64 fnv64(const void *in, u32 len) {
  const u64 prime = 0x100000001B3;
  u64 result = 0xcbf29ce484222325;

  const u8 *p = (const u8 *)in;
  for (u32 i = 0; i < len; i++) {
    result = (result ^ p[i]) * prime;
  }

  return result;
}