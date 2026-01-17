/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#define SN_STD_ARCH_AMD64 (1)
#define SN_STD_ARCH_AARCH64 (2)
#define SN_STD_ARCH_WASM32 (3)

#ifndef SN_STD_ARCH
#error "SN_STD_ARCH is not defined"
#endif

#define SN_STD_SYSTEM_WINDOWS_DESKTOP (1)
#define SN_STD_SYSTEM_WINDOWS_UNIVERSAL (2)
#define SN_STD_SYSTEM_LINUX (3)
#define SN_STD_SYSTEM_ANDROID (4)
#define SN_STD_SYSTEM_EMSCRIPTEN (5)

#ifndef SN_STD_SYSTEM
#error "SN_STD_SYSTEM is not defined"
#endif

#define SN_STD_SYSTEM_WINDOWS                        \
  (SN_STD_SYSTEM == SN_STD_SYSTEM_WINDOWS_DESKTOP || \
   SN_STD_SYSTEM == SN_STD_SYSTEM_WINDOWS_UNIVERSAL)

#define SN_STD_SYSTEM_HAS_PTHREADS           \
  (SN_STD_SYSTEM == SN_STD_SYSTEM_LINUX ||   \
   SN_STD_SYSTEM == SN_STD_SYSTEM_ANDROID || \
   SN_STD_SYSTEM == SN_STD_SYSTEM_EMSCRIPTEN)
