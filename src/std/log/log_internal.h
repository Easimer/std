/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Chronometry.h"
#include "std/Types.h"
#include "std/log.h"

#include <stdarg.h>

extern const char *log_level_strings[6];
extern const char *log_level_colors[6];

struct log_event {
  const char *fmt;
  const char *file;
  struct chrono_date time;
  int32_t line;
  enum log_level level;
};

typedef void (*log_pfn_on_event)(const struct log_handler *handler,
                                 const struct log_event *ev,
                                 va_list ap);

struct log_handler_api {
  log_pfn_on_event on_event;
};

struct log_handler {
  const struct log_handler_api *api;
  void *data;
};

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)
#define SN_LOG_INITIALIZER2_(f, p)                         \
  static void f(void);                                     \
  __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
  __pragma(comment(linker, "/include:" p #f "_")) static void f(void)
#define SN_LOG_INITIALIZER(f) SN_LOG_INITIALIZER2_(f, "")
#else
#define SN_LOG_INITIALIZER(f)                       \
  static void f(void) __attribute__((constructor)); \
  static void f(void)
#endif

/**
 * \brief A function that initializes a log handler.
 * \param handler Pointer to an initialized `struct log_handler` value
 * \returns `LOG_ERR_OK` if successfully initialized, or an error code
 * otherwise. The handler will only be registered if this function succeeds.
 */
typedef int (*log_pfn_handler_init)(const struct log_handler *handler);

/**
 * Defines a log handler that will be registered at startup.
 * \param Name The (globally unique) name of the handler
 * \param Api Pointer to a `struct log_handler_api`
 * \param Data Pointer to the user-defined data of the handler
 * \param Init Pointer to a `log_pfn_handler_init` that initializes the handler
 */
#define SN_LOG_HANDLER_DEFINE(Name, Api, Data, Init)      \
  static const struct log_handler handler_##Name = {      \
      .api = (Api),                                       \
      .data = (Data),                                     \
  };                                                      \
                                                          \
  SN_LOG_INITIALIZER(sn_log_handler_initializer_##Name) { \
    int rc = Init(&handler_##Name);                       \
    if (rc == LOG_ERR_OK) {                               \
      log_register_handler(&handler_##Name);              \
    }                                                     \
  }

