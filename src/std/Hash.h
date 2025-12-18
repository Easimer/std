/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Types.h"

#if __cplusplus
extern "C" {
#endif

// Fowler-Noll-Vo hash, FNV-1a variant
u64 fnv64(const void *in, u32 len);

#if __cplusplus
}
#endif