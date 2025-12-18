/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Slice.hpp"

#include <string>

inline Slice<const char> sliceFromStdStr(const std::string &s) {
  return {s.data(), u32(s.size())};
}

inline std::string stdStrFrom(Slice<const u8> s) {
  return {(const char *)s.data, (const char *)s.data + s.length};
}
