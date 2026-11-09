/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/SliceUtils.hpp"
#include "std/serializer/Serializer2.hpp"

struct SliceReader final : sn::ISerializerStream {
  Slice<u8> src;

  SliceReader() = default;
  SliceReader(Slice<u8> src) : src(src) {}

  Optional<Error> read(MutSlice<u8> buffer) noexcept override {
    if (src.length < buffer.length) {
      return Error{.type = &sn::ERROR_SERIALIZER_PAST_END_OF_STREAM};
    }

    Slice<u8> sub = src.subarray(0, buffer.length);
    buffer.copy(sub);

    src.shrinkFromLeftByCount(buffer.length);
    return {};
  }

  Optional<Error> write(Slice<u8> buffer) noexcept override {
    (void)buffer;
    DCHECK("Cannot be used for serialization");
    return Error{.type = &sn::ERROR_SERIALIZER_INVALID_OPERATION};
  }
};
