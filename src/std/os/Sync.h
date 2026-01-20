/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <std/Arena.h>
#include <std/Types.h>

typedef struct Barrier Barrier;

#ifdef __cplusplus
extern "C" {
#endif

Barrier *barrierCreate(Arena *arena, u32 count);
void barrierSync(Barrier *barrier);
void barrierDestroy(Barrier *barrier);

#ifdef __cplusplus
}
#endif