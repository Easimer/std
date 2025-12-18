/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#if defined(_MSC_VER)
#define DEBUGBREAK() __debugbreak()
#elif __has_builtin(__builtin_debugtrap)
#define DEBUGBREAK() __builtin_debugtrap()
#elif defined(__GNUC__) || defined(__clang__)
#define DEBUGBREAK() __builtin_trap()
#else
#define DEBUGBREAK()
#endif

#if __cplusplus
extern "C" {
#endif

#if __cplusplus
#if defined(_MSC_VER)
#define CHECK_NORETURN [[noreturn]] __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
#define CHECK_NORETURN [[noreturn]] __attribute__((noreturn))
#else
#define CHECK_NORETURN [[noreturn]]
#endif
#else
#if defined(_MSC_VER)
#define CHECK_NORETURN __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
#define CHECK_NORETURN __attribute__((noreturn))
#else
#define CHECK_NORETURN
#endif
#endif

void checkFail(const char *pExpr, const char *pFile, unsigned line);

#if __cplusplus
}
#endif

#define CHECK(expression)                                             \
  (void)((!!(expression)) ||                                          \
         (checkFail((#expression), (__FILE__), (unsigned)(__LINE__)), \
          DEBUGBREAK(), 0))

#ifdef NDEBUG
#define DCHECK(expr) ((void)0)
#else
#define DCHECK(expr) CHECK(expr)
#endif

#define IMPLICATION(p, q) (!(p) || (q))

#define TODO() checkFail("TODO()", (__FILE__), (unsigned)(__LINE__))
#define NOTREACHED() checkFail("NOTREACHED()", (__FILE__), (unsigned)(__LINE__))

#if defined(_MSC_VER)
#define SN_ASSUME(expr) __assume(expr)
#elif defined(__clang__)
#define SN_ASSUME(expr) __builtin_assume(expr)
#elif defined(__CGUNC__)
#define SN_ASSUME(expr)        \
  do {                         \
    if (!(expr))               \
      __builtin_unreachable(); \
  } while (0)
#else
#define SN_ASSUME(expr)
#endif
