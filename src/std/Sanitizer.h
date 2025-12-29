/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#if _MSC_VER
// On MSVC, check only __SANITIZE_ADDRESS__ because __has_feature() triggers a
// warning
#if defined(__SANITIZE_ADDRESS__)
#define SN_ASAN_ACTIVE
#endif
#else
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define SN_ASAN_ACTIVE
#endif
#endif

#if defined(SN_ASAN_ACTIVE)

#include <stddef.h>

#if __cplusplus
extern "C" {
#endif
// Marks memory region [addr, addr+size) as unaddressable.
// This memory must be previously allocated by the user program. Accessing
// addresses in this region from instrumented code is forbidden until
// this region is unpoisoned. This function is not guaranteed to poison
// the whole region - it may poison only subregion of [addr, addr+size) due
// to ASan alignment restrictions.
// Method is NOT thread-safe in the sense that no two threads can
// (un)poison memory in the same memory region simultaneously.
void __asan_poison_memory_region(void const volatile *addr, size_t size);
// Marks memory region [addr, addr+size) as addressable.
// This memory must be previously allocated by the user program. Accessing
// addresses in this region is allowed until this region is poisoned again.
// This function may unpoison a superregion of [addr, addr+size) due to
// ASan alignment restrictions.
// Method is NOT thread-safe in the sense that no two threads can
// (un)poison memory in the same memory region simultaneously.
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
#if __cplusplus
}
#endif

#define SN_ASAN_POISON(addr, size) __asan_poison_memory_region((addr), (size))
#define SN_ASAN_UNPOISON(addr, size) \
  __asan_unpoison_memory_region((addr), (size))
#else
#define SN_ASAN_POISON(addr, size) ((void)(addr), (void)(size))
#define SN_ASAN_UNPOISON(addr, size) ((void)(addr), (void)(size))
#endif /* defined(SN_ASAN_ACTIVE) */
