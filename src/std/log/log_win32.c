/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Check.h"
#include "std/log/log_internal.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stddef.h>
#include <strsafe.h>

#define RING_SIZE 8192

struct log_handler_win32_data {
  CRITICAL_SECTION lock;
  HANDLE std_output;
  HANDLE wake_event;
  HANDLE worker_thread;
  volatile LONG shutdown_flag;

  CRITICAL_SECTION ring_lock;
  char ring[RING_SIZE];
  size_t head;
  size_t tail;
};

static DWORD WINAPI threadproc_log_worker(LPVOID param) {
  struct log_handler_win32_data *d = param;
  char tmp[1024];
  for (;;) {
    WaitForSingleObject(d->wake_event, INFINITE);
    if (InterlockedCompareExchange(&d->shutdown_flag, 0, 0))
      break;

    for (;;) {
      size_t used;
      size_t n;
      EnterCriticalSection(&d->ring_lock);
      used = (d->head - d->tail) & (RING_SIZE - 1);
      if (used == 0) {
        LeaveCriticalSection(&d->ring_lock);
        break;
      }
      n = used < sizeof(tmp) ? used : sizeof(tmp);
      if (d->tail + n <= RING_SIZE) {
        memcpy(tmp, d->ring + d->tail, n);
      } else {
        size_t p1 = RING_SIZE - d->tail;
        memcpy(tmp, d->ring + d->tail, p1);
        memcpy(tmp + p1, d->ring, n - p1);
      }
      d->tail = (d->tail + n) & (RING_SIZE - 1);
      LeaveCriticalSection(&d->ring_lock);

      DWORD cb = 0;
      WriteFile(d->std_output, tmp, (DWORD)n, &cb, NULL);
    }
  }
  return 0;
}

static void enqueue(struct log_handler_win32_data *d,
                    const char *buf,
                    size_t len) {
  EnterCriticalSection(&d->ring_lock);
  while (len) {
    size_t used = (d->head - d->tail) % RING_SIZE;
    size_t free = (RING_SIZE - 1) - used;
    if (free == 0)
      break;
    size_t n = len < free ? len : free;
    if (d->head + n <= RING_SIZE) {
      memcpy(d->ring + d->head, buf, n);
    } else {
      size_t p1 = RING_SIZE - d->head;
      memcpy(d->ring + d->head, buf, p1);
      memcpy(d->ring, buf + p1, n - p1);
    }
    d->head = (d->head + n) % RING_SIZE;
    buf += n;
    len -= n;
  }
  LeaveCriticalSection(&d->ring_lock);
  SetEvent(d->wake_event);
}

static void log_on_event(const struct log_handler *handler,
                         const struct log_event *ev,
                         va_list ap) {
  static char buf[1024];

  HRESULT hr;

  const char *level = log_level_strings[ev->level];

  struct log_handler_win32_data *data = handler->data;
  EnterCriticalSection(&data->lock);

  STRSAFE_LPSTR end = NULL;

  hr = StringCchPrintfExA(buf, sizeof(buf), &end, NULL, 0,
                          "%02d:%02d:%02d %-5s %s:%d: ", ev->time.hour,
                          ev->time.minute, ev->time.second, level, ev->file,
                          ev->line);
  CHECK(hr == S_OK || hr == STRSAFE_E_INSUFFICIENT_BUFFER);

  OutputDebugStringA(buf);
  if (data->std_output != NULL) {
    ptrdiff_t c = end - buf;
    enqueue(data, buf, c);
  }

  hr = StringCchVPrintfExA(buf, sizeof(buf), &end, NULL, 0, ev->fmt, ap);
  CHECK(hr == S_OK || hr == STRSAFE_E_INSUFFICIENT_BUFFER);
  OutputDebugStringA(buf);
  OutputDebugStringA("\n");

  if (data->std_output != NULL) {
    ptrdiff_t c = end - buf;
    enqueue(data, buf, c);
    enqueue(data, "\n", 1);
  }

  LeaveCriticalSection(&data->lock);
}

static void log_win32_shutdown(const struct log_handler *handler) {
  struct log_handler_win32_data *data = handler->data;
  InterlockedExchange(&data->shutdown_flag, 1);
  SetEvent(data->wake_event);
  WaitForSingleObject(data->worker_thread, INFINITE);
  CloseHandle(data->worker_thread);
  CloseHandle(data->wake_event);
  DeleteCriticalSection(&data->ring_lock);
  DeleteCriticalSection(&data->lock);
}

static const struct log_handler_api api = {
    .on_event = log_on_event,
    .shutdown = log_win32_shutdown,
};

static struct log_handler_win32_data data;

static int log_handler_win32_init(const struct log_handler *handler) {
  struct log_handler_win32_data *data = handler->data;

  InitializeCriticalSection(&data->lock);
  InitializeCriticalSection(&data->ring_lock);
  data->std_output = NULL;
  data->wake_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  data->shutdown_flag = 0;

  HANDLE std_output = GetStdHandle(STD_OUTPUT_HANDLE);
  if (std_output != NULL && std_output != INVALID_HANDLE_VALUE) {
    data->std_output = std_output;
    data->worker_thread =
        CreateThread(NULL, 0, threadproc_log_worker, data, 0, NULL);
    SetThreadDescription(data->worker_thread, L"Log worker");
  }

  return LOG_ERR_OK;
}

SN_LOG_HANDLER_DEFINE(win32, &api, &data, log_handler_win32_init);
