/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Slice.hpp"

#include <vector>

/**
 * @deprecated Prefer sliceFromStd instead
 */
template <typename T>
Slice<T> sliceFrom(const std::vector<T> &in) {
  CHECK(in.size() <= 0xFFFFFFFF);
  T *data = const_cast<T *>(in.data());
  return {data, (u32)in.size()};
}

template <typename T>
std::vector<T> stdVectorFrom(Slice<T> in) {
  if (in.data == nullptr || in.length == 0) {
    return {};
  }

  return {in.data, in.data + in.length};
}
