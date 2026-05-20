/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Check.h"
#include "std/log/log_internal.h"

#include <android/log.h>
#include <stdio.h>
#include <string.h>

static android_LogPriority mapPrio(enum log_level level) {
  switch (level) {
    case LOG_TRACE:
      return ANDROID_LOG_DEBUG;
    case LOG_DEBUG:
      return ANDROID_LOG_DEBUG;
    case LOG_INFO:
      return ANDROID_LOG_INFO;
    case LOG_WARN:
      return ANDROID_LOG_WARN;
    case LOG_ERROR:
      return ANDROID_LOG_ERROR;
    case LOG_FATAL:
      return ANDROID_LOG_FATAL;
    default:
      return ANDROID_LOG_DEFAULT;
  }
}

static void log_on_event(const struct log_handler *handler,
                         const struct log_event *ev,
                         va_list ap) {
  android_LogPriority prio = mapPrio(ev->level);
  char tag[512];
  char message[512];
  memset(tag, 0, 512);
  memset(message, 0, 512);

  snprintf(tag, 511, "%s:%d: ", ev->file, ev->line);

  vsnprintf(message, 511, ev->fmt, ap);

  __android_log_write(prio, tag, message);
}

static const struct log_handler_api api = {
    .on_event = log_on_event,
};

static int log_handler_android_init(const struct log_handler *handler) {
  return LOG_ERR_OK;
}

SN_LOG_HANDLER_DEFINE(android, &api, NULL, log_handler_android_init);
