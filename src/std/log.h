/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdarg.h>
#include "std/os/OsInfo.h"

#ifndef LOG_HANDLER_MAX_COUNT
#define LOG_HANDLER_MAX_COUNT 16
#endif

#ifndef LOG_HANDLER_WIN32_ENABLED
#define LOG_HANDLER_WIN32_ENABLED SN_STD_SYSTEM_WINDOWS
#endif

#ifndef LOG_HANDLER_LINUX_ENABLED
#define LOG_HANDLER_LINUX_ENABLED SN_STD_SYSTEM == SN_STD_SYSTEM_LINUX
#endif

#ifndef LOG_HANDLER_ANDROID_ENABLED
#define LOG_HANDLER_ANDROID_ENABLED SN_STD_SYSTEM == SN_STD_SYSTEM_ANDROID
#endif

#ifndef LOG_HANDLER_EMSCRIPTEN_ENABLED
#define LOG_HANDLER_EMSCRIPTEN_ENABLED SN_STD_SYSTEM == SN_STD_SYSTEM_EMSCRIPTEN
#endif

#define LOG_ERR_OK 0
#define LOG_ERR_ENOENT 2
#define LOG_ERR_EINVAL 22

struct log_handler;

enum log_level {
  LOG_TRACE,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
};

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#if defined(_MSC_VER) && defined(_PREFAST_)
#ifdef _USE_ATTRIBUTES_FOR_SAL
#undef _USE_ATTRIBUTES_FOR_SAL
#endif
#define _USE_ATTRIBUTES_FOR_SAL 1
#include <sal.h>
#define SN_STD_FMTSTR _Printf_format_string_
#else
#define SN_STD_FMTSTR
#endif

#if defined(__MINGW32__) && !defined(__clang__)
#define SN_STD_FMTARGS(FMT) __attribute__((format(gnu_printf, FMT, FMT + 1)))
#elif (defined(__clang__) || defined(__GNUC__))
#define SN_STD_FMTARGS(FMT) __attribute__((format(printf, FMT, FMT + 1)))
#else
#define SN_STD_FMTARGS(FMT)
#endif

#if __cplusplus
extern "C" {
#endif

void log_log(enum log_level level,
             const char *file,
             int line,
             SN_STD_FMTSTR const char *fmt,
             ...) SN_STD_FMTARGS(4);

int log_register_handler(const struct log_handler *handler);
int log_unregister_handler(const struct log_handler *handler);

void log_shutdown(void);

#if __cplusplus
}
#endif
