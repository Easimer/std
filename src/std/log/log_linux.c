/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Check.h"
#include "std/log/log_internal.h"

#include <stdio.h>
#include <string.h>

static FILE *file_from_level(enum log_level level) {
  switch (level) {
    case LOG_TRACE:
    case LOG_DEBUG:
    case LOG_ERROR:
    case LOG_FATAL:
      return stderr;
    case LOG_INFO:
    case LOG_WARN:
      return stdout;
    default:
      return stdout;
  }
}

static void log_on_event(const struct log_handler *handler,
                         const struct log_event *ev,
                         va_list ap) {
  FILE *file = file_from_level(ev->level);
  const char *level = log_level_strings[ev->level];

  fprintf(file, "%02d:%02d:%02d %-5s %s:%d: ", ev->time.hour, ev->time.minute,
          ev->time.second, level, ev->file, ev->line);
  vfprintf(file, ev->fmt, ap);
  fprintf(file, "\n");
}

static const struct log_handler_api api = {
    .on_event = log_on_event,
};

static int log_handler_linux_init(const struct log_handler *handler) {
  return LOG_ERR_OK;
}

SN_LOG_HANDLER_DEFINE(linux, &api, NULL, log_handler_linux_init);
