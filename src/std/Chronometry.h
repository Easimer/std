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

typedef struct TimePoint_t *TimePoint;

TimePoint chrono_getCurrentTime();
/**
 * Computes the amount of time that has passed between t0 and t1, i.e. `t1 -
 * t0`.
 * @returns Number of seconds
 */
f64 chrono_secondsBetween(TimePoint t0, TimePoint t1);

#if __cplusplus
}
#endif