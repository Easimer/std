/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/log.h"
#include "std/Check.h"
#include "std/Chronometry.h"
#include "std/CompilerInfo.h"
#include "std/log/log_internal.h"
#include "std/os/OsInfo.h"

#include <string.h>

struct log_handler_slot {
  const struct log_handler *handler;
};

static struct log_handler_slot handler_registry[LOG_HANDLER_MAX_COUNT];
static uint32_t handler_registry_len = 0;

const char *log_level_strings[] = {"TRACE", "DEBUG", "INFO",
                                   "WARN",  "ERROR", "FATAL"};
const char *log_level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m",
                                  "\x1b[33m", "\x1b[31m", "\x1b[35m"};

void log_log(enum log_level level,
             const char *file,
             int line,
             SN_STD_FMTSTR const char *fmt,
             ...) {
  CHECK(0 <= level && level < 6);

  struct chrono_date t = chrono_get_local_date();

  struct log_event ev = {
      .fmt = fmt,
      .file = file,
      .time = t,
      .line = line,
      .level = level,
  };

  va_list ap;
  va_start(ap, fmt);
  for (uint32_t i = 0; i < handler_registry_len; i++) {
    const struct log_handler *handler = handler_registry[i].handler;
    handler->api->on_event(handler, &ev, ap);
  }
  va_end(ap);
}

int log_register_handler(const struct log_handler *handler) {
  if (handler == NULL) {
    return LOG_ERR_EINVAL;
  }
  if (handler_registry_len == LOG_HANDLER_MAX_COUNT) {
    return LOG_ERR_ENOENT;
  }

  struct log_handler_slot *slot = &handler_registry[handler_registry_len];
  memset(slot, 0, sizeof(struct log_handler_slot));

  slot->handler = handler;

  handler_registry_len++;

  return LOG_ERR_OK;
}

int log_unregister_handler(const struct log_handler *handler) {
  if (handler_registry_len == 0) {
    return LOG_ERR_OK;
  }

  struct log_handler_slot *const last_slot =
      &handler_registry[handler_registry_len - 1];

  for (uint32_t i = 0; i < handler_registry_len; i++) {
    struct log_handler_slot *slot = &handler_registry[i];
    if (slot->handler != handler) {
      continue;
    }

    // Move contents of the last slot into the slot to be removed
    memset(slot, 0, sizeof(*slot));
    *slot = *last_slot;
    // Pop the last slot
    memset(last_slot, 0, sizeof(*last_slot));
    handler_registry_len--;

    return LOG_ERR_OK;
  }

  return LOG_ERR_ENOENT;
}

#if LOG_HANDLER_WIN32_ENABLED
#include "log_win32.c"
#elif LOG_HANDLER_ANDROID_ENABLED
#include "log_android.c"
#elif LOG_HANDLER_EMSCRIPTEN_ENABLED
#include "log_emscripten.c"
#elif LOG_HANDLER_LINUX_ENABLED
#include "log_linux.c"
#endif
