/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "./Testing.hpp"
#include "./Types.h"

struct SnTestRegistry {
  struct SnTestChain *first;
  struct SnTestChain *prev;
};

static struct SnTestRegistry gSnTestRegistry;
static u32 gSnTestRegistryInit = 0;

SN_STD_API void snTestRegister(struct SnTestChain *test) {
  if (gSnTestRegistryInit == 0) {
    gSnTestRegistryInit = 1;

    gSnTestRegistry.first = NULL;
    gSnTestRegistry.prev = NULL;
  }

  if (gSnTestRegistry.first != NULL) {
    gSnTestRegistry.prev->next = test;
    gSnTestRegistry.prev = test;
  } else {
    gSnTestRegistry.first = gSnTestRegistry.prev = test;
  }
}

SN_STD_API struct SnTestChain *snTestGetFirst(void) {
  if (gSnTestRegistryInit == 0) {
    return NULL;
  }

  return gSnTestRegistry.first;
}
