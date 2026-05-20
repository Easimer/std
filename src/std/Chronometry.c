/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Chronometry.h"
#include "std/Types.h"
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

struct chrono_date chrono_get_local_date(void) {
  SYSTEMTIME st;
  memset(&st, 0, sizeof(st));

  GetLocalTime(&st);

  struct chrono_date ret = {
      .year = st.wYear,
      .month = st.wMonth,
      .day = st.wDay,
      .hour = st.wHour,
      .minute = st.wMinute,
      .second = st.wSecond,
      .milliseconds = st.wMilliseconds,
  };
  return ret;
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

struct chrono_date chrono_get_local_date(void) {
  time_t t = time(NULL);
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  localtime_r(&t, &tm);

  struct chrono_date ret = {
      .year = 1900 + tm.tm_year,
      .month = 1 + tm.tm_mon,
      .day = tm.tm_mday,
      .hour = tm.tm_hour,
      .minute = tm.tm_min,
      .second = tm.tm_sec,
      .milliseconds = 0,
  };
  return ret;
}
#endif
