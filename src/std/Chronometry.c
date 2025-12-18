/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Chronometry.h"
#include <stdlib.h>
#include <string.h>

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#endif

#if ANDROID
#include <time.h>
#endif

#if EMSCRIPTEN
#include <emscripten/html5.h>
#endif

#if _WIN32
TimePoint chrono_getCurrentTime() {
  TimePoint ret = 0;

  LARGE_INTEGER li;
  if (!QueryPerformanceCounter(&li)) {
    abort();
  }

  memcpy(&ret, &li.QuadPart, sizeof(li.QuadPart));
  return ret;
}

f64 chrono_secondsBetween(TimePoint t0, TimePoint t1) {
  LARGE_INTEGER li0, li1, freq;
  memcpy(&li0.QuadPart, &t0, sizeof(TimePoint));
  memcpy(&li1.QuadPart, &t1, sizeof(TimePoint));
  i64 delta = li1.QuadPart - li0.QuadPart;

  if (!QueryPerformanceFrequency(&freq)) {
    abort();
  }

  return (f64)delta / ((f64)freq.QuadPart);
}

#else
TimePoint chrono_getCurrentTime() {
  TimePoint ret = 0;
#if EMSCRIPTEN
  u32 ticks;
#else
  u64 ticks;
#endif

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  ticks = now.tv_sec;
  ticks *= 1000000000;
  ticks += now.tv_nsec;

  memcpy(&ret, &ticks, sizeof(ticks));
  return ret;
}

f64 chrono_secondsBetween(TimePoint t0, TimePoint t1) {
  u64 ticks0, ticks1;
  memcpy(&ticks0, &t0, sizeof(t0));
  memcpy(&ticks1, &t1, sizeof(t1));
  i64 delta = ticks1 - ticks0;

  return (f64)delta / (f64)1000000000;
}
#endif
