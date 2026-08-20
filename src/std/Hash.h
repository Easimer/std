/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Types.h"

#if __cplusplus
extern "C" {
#endif

// Fowler-Noll-Vo hash, FNV-1a variant
u64 fnv64(const void *in, size_t len);

u64 rpdh_nano(const void *in, size_t len);
u64 rpdh_micro(const void *in, size_t len);
u64 rpdh(const void *in, size_t len);

#if __cplusplus
}
#endif

#if __cplusplus
#include "std/Slice.hpp"

template <typename T>
static inline u64 hashFNV64(Slice<T> data) {
  return fnv64(data.data, data.byteLength());
}

template <typename T>
static inline u64 hashRapidNano(Slice<T> data) {
  return rpdh_nano(data.data, data.byteLength());
}

template <typename T>
static inline u64 hashRapidMicro(Slice<T> data) {
  return rpdh_micro(data.data, data.byteLength());
}

template <typename T>
static inline u64 hashRapid(Slice<T> data) {
  return rpdh(data.data, data.byteLength());
}
#endif
