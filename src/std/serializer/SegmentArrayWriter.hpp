/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/SegmentArray.hpp"
#include "std/serializer/Serializer2.hpp"

struct SegmentArrayWriter final : sn::ISerializerStream {
  SegmentArray<u8> dst;

  SegmentArrayWriter() = default;
  SegmentArrayWriter(Arena *arena) : dst(arena) {}

  Optional<Error> read(MutSlice<u8> buffer) noexcept override {
    (void)buffer;
    DCHECK("Cannot be used for deserialization");
    return Error{.type = &sn::ERROR_SERIALIZER_INVALID_OPERATION};
  }

  Optional<Error> write(Slice<u8> buffer) noexcept override {
    dst.push(buffer);
    return {};
  }
};
