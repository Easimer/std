/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "./CompilerInfo.h"

#if SN_MSVC
#define SN_DLLEXPORT __declspec(dllexport)
#define SN_DLLIMPORT __declspec(dllimport)
#define SN_WEAK_SYMBOL
#elif SN_GCC || SN_CLANG
#define SN_DLLEXPORT __attribute__((visibility("default")))
#define SN_DLLIMPORT
#define SN_WEAK_SYMBOL __attribute__((weak))
#else
#define SN_DLLEXPORT
#define SN_DLLIMPORT
#define SN_WEAK_SYMBOL
#endif

