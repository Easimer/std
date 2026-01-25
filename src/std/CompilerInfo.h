/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#define SN_COMPILER_UNKNOWN 0
#define SN_COMPILER_CLANG 1
#define SN_COMPILER_GCC 2
#define SN_COMPILER_MSVC 3

#if defined(__clang__)
#define SN_COMPILER SN_COMPILER_CLANG
#define SN_CLANG 1
#define SN_GCC 0
#define SN_MSVC 0

#define SN_FORCEINLINE __attribute__((always_inline)) inline
#define SN_STD_WEAK_SYMBOL __attribute__((weak))

#elif defined(__GNUC__) || defined(__GNUG__)
#define SN_COMPILER SN_COMPILER_GCC
#define SN_CLANG 0
#define SN_GCC 1
#define SN_MSVC 0

#define SN_FORCEINLINE __attribute__((always_inline)) inline
#define SN_STD_WEAK_SYMBOL __attribute__((weak))

#elif defined(_MSC_VER)
#define SN_COMPILER SN_COMPILER_MSVC
#define SN_CLANG 0
#define SN_GCC 0
#define SN_MSVC 1

#define SN_FORCEINLINE __forceinline
#define SN_STD_WEAK_SYMBOL

#else
#define SN_COMPILER SN_COMPILER_UNKNOWN
#define SN_CLANG 0
#define SN_GCC 0
#define SN_MSVC 0

#define SN_FORCEINLINE inline
#define SN_STD_WEAK_SYMBOL
#endif
