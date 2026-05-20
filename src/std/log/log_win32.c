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
#include <strsafe.h>

struct log_handler_win32_data {
  CRITICAL_SECTION lock;
  HANDLE std_output;
};

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
    DWORD cbWritten = 0;
    WriteConsoleA(data->std_output, buf, (DWORD)c, &cbWritten, NULL);
  }

  hr = StringCchVPrintfExA(buf, sizeof(buf), &end, NULL, 0, ev->fmt, ap);
  CHECK(hr == S_OK || hr == STRSAFE_E_INSUFFICIENT_BUFFER);
  OutputDebugStringA(buf);
  OutputDebugStringA("\n");

  if (data->std_output != NULL) {
    ptrdiff_t c = end - buf;
    DWORD cbWritten = 0;
    WriteConsoleA(data->std_output, buf, (DWORD)c, &cbWritten, NULL);

    cbWritten = 0;
    WriteConsoleA(data->std_output, "\n", 1, &cbWritten, NULL);
  }

  LeaveCriticalSection(&data->lock);
}

static const struct log_handler_api api = {
    .on_event = log_on_event,
};

static struct log_handler_win32_data data;

static int log_handler_win32_init(const struct log_handler *handler) {
  struct log_handler_win32_data *data = handler->data;

  InitializeCriticalSection(&data->lock);
  data->std_output = NULL;

  HANDLE std_output = GetStdHandle(STD_OUTPUT_HANDLE);
  if (std_output != NULL && std_output != INVALID_HANDLE_VALUE) {
    data->std_output = std_output;
  }

  return LOG_ERR_OK;
}

SN_LOG_HANDLER_DEFINE(win32, &api, &data, log_handler_win32_init);
