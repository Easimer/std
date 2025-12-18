/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/CompilerInfo.h"

#if defined(SN_ASAN_ACTIVE) && SN_MSVC
#define SN_MSVC_ASAN
#endif

#if defined(SN_MSVC_ASAN)
#if __cplusplus
extern "C" {
#endif
void __asan_poison_memory_region(void const volatile *addr, size_t size);
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
#if __cplusplus
}
#endif

#define SN_ASAN_POISON(addr, size) __asan_poison_memory_region((addr), (size))
#define SN_ASAN_UNPOISON(addr, size) \
  __asan_unpoison_memory_region((addr), (size))
#else
#define SN_ASAN_POISON(addr, size)
#define SN_ASAN_UNPOISON(addr, size)
#endif
