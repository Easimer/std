/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Optional.hpp"
#include "std/serializer/Serializer2.hpp"

namespace sn {

template <typename T>
[[nodiscard]] inline Optional<Error> serialize(Serialization *S,
                                               Optional<T> *maybe) {
  if (S->isWriting) {
    u8 hasValueTag = maybe->hasValue() ? 0x01 : 0x00;
    SN_TRY_SERIALIZE(serialize(S, &hasValueTag));
    if (hasValueTag != 0x00) {
      SN_TRY_SERIALIZE(serialize(S, &maybe->value()));
    }
  } else {
    u8 hasValueTag;
    SN_TRY_SERIALIZE(serialize(S, &hasValueTag));
    if (hasValueTag != 0x00) {
      *maybe = T();
      SN_TRY_SERIALIZE(serialize(S, &maybe->value()));
    } else {
      maybe->reset();
    }
  }

  return {};
}
}  // namespace sn