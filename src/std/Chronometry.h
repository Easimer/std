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

typedef struct chrono_date {
  // Year
  i16 year;
  // Month, 1..12
  i16 month;
  // Day, 1..31
  i16 day;
  // Hour, 0..23
  i16 hour;
  // Minute, 0..59
  i16 minute;
  // Second, 0..59
  i16 second;
  // Milliseconds, 0..999
  i16 milliseconds;
} chrono_date;

struct chrono_date chrono_get_local_date(void);

#if __cplusplus
}
#endif