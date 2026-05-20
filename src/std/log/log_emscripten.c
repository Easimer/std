/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Check.h"
#include "std/log/log_internal.h"

#include <emscripten/console.h>

#include <stdio.h>

static void log_on_event(const struct log_handler *handler,
                         const struct log_event *ev,
                         va_list ap) {
  const char *level = log_level_strings[ev->level];

  char temp[1024];
  temp[vsnprintf(temp, 1023, ev->fmt, ap)] = '\0';
  switch (ev->level) {
    case LOG_ERROR:
      emscripten_console_errorf("%s:%d: %s", ev->file, ev->line, temp);
      break;
    case LOG_WARN:
      emscripten_console_warnf("%s:%d: %s", ev->file, ev->line, temp);
      break;
    case LOG_TRACE:
      emscripten_console_tracef("%-5s %s:%d: %s", level, ev->file, ev->line,
                                temp);
      break;
    default:
      emscripten_console_logf("%-5s %s:%d: %s", level, ev->file, ev->line,
                              temp);
      break;
  }
}

static const struct log_handler_api api = {
    .on_event = log_on_event,
};

static int log_handler_emscripten_init(const struct log_handler *handler) {
  return LOG_ERR_OK;
}

SN_LOG_HANDLER_DEFINE(emscripten, &api, NULL, log_handler_emscripten_init);
