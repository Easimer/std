/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/serializer/Serializer2.hpp"
#include "std/Endian.h"
#include "std/VU128.h"

namespace sn {

const ErrorType ERROR_SERIALIZER_INVALID_OPERATION = {
    .title = "The serialization stream doesn't support this operation",
};

const ErrorType ERROR_SERIALIZER_PAST_END_OF_STREAM = {
    .title = "Deserialization went past the end of the stream",
};

const ErrorType ERROR_SERIALIZER_NOT_ENOUGH_SPACE = {
    .title = "Not enough space is available to (de)serialize",
};

[[nodiscard]] Optional<Error> serializeBytes(Serialization *S,
                                             u8 *value,
                                             size_t count) {
  if (S->isWriting) {
    Slice<u8> view = {value, count};
    Optional<Error> err = S->stream->write(view);
    if (err) {
      return err;
    }
  } else {
    MutSlice<u8> view = {value, count};
    Optional<Error> err = S->stream->read(view);
    if (err) {
      return err;
    }
  }

  return {};
}

template <typename T>
static inline Optional<Error> serializePrimitives(Serialization *S,
                                                  T *value,
                                                  size_t count) {
  if (S->isWriting) {
    Slice<u8> view = {reinterpret_cast<const u8 *>(value), count * sizeof(T)};
    Optional<Error> err = S->stream->write(view);
    if (err) {
      return err;
    }
  } else {
    MutSlice<u8> view = {reinterpret_cast<u8 *>(value), count * sizeof(T)};
    Optional<Error> err = S->stream->read(view);
    if (err) {
      return err;
    }
  }

  return {};
}

template <typename T>
inline Optional<Error> serializePrimitive(Serialization *S, T *value) {
  return serializePrimitives(S, value, 1);
}

Optional<Error> serialize(Serialization *S,
                          u32 *value,
                          const struct SerializeVU128Tag &) {
  u8 buf[5];
  if (S->isWriting) {
    u32 len = vu128_encode(buf, *value);
    Optional<Error> err = S->stream->write(Slice<u8>(buf, len));
    if (err) {
      return err;
    }
  } else {
    Optional<Error> err = S->stream->read(MutSlice<u8>(&buf[0], 1));
    if (err) {
      return err;
    }
    u32 len = vu128_begin_decode(buf);
    if (len > 4) {
      return Error{
          .type = &ERROR_SERIALIZER_NOT_ENOUGH_SPACE,
          .detail = "Cannot decode VU128 value into a u32",
      };
    }
    err = S->stream->read(MutSlice<u8>(&buf[1], len));
    if (err) {
      return err;
    }
    *value = vu128_decode(buf);
  }

  return {};
}

Optional<Error> serialize(Serialization *S, u8 *value) {
  return serializePrimitive<u8>(S, value);
}

Optional<Error> serialize(Serialization *S, u16 *value) {
  return serializePrimitive<u16>(S, value);
}

Optional<Error> serialize(Serialization *S, u32 *value) {
  return serializePrimitive<u32>(S, value);
}

Optional<Error> serialize(Serialization *S, u64 *value) {
  return serializePrimitive<u64>(S, value);
}

Optional<Error> serialize(Serialization *S, i8 *value) {
  return serializePrimitive<i8>(S, value);
}

Optional<Error> serialize(Serialization *S, i16 *value) {
  return serializePrimitive<i16>(S, value);
}

Optional<Error> serialize(Serialization *S, i32 *value) {
  return serializePrimitive<i32>(S, value);
}

Optional<Error> serialize(Serialization *S, i64 *value) {
  return serializePrimitive<i64>(S, value);
}

Optional<Error> serialize(Serialization *S, f32 *value) {
  return serializePrimitive<f32>(S, value);
}

Optional<Error> serialize(Serialization *S, f64 *value) {
  return serializePrimitive<f64>(S, value);
}

#if __EMSCRIPTEN__
Optional<Error> serialize(Serialization *S, size_t *value) {
  if (S->isWriting) {
    u64 v = *value;
    return serialize(S, &v);
  } else {
    u64 v = 0;

    SN_TRY_SERIALIZE(serialize(S, &v));

    if constexpr (sizeof(size_t) == sizeof(u8)) {
      if (v > 0xFF) {
        return Error{
            .type = &ERROR_SERIALIZER_NOT_ENOUGH_SPACE,
            .detail =
                "Cannot deserialize size_t value into this platform's size_t "
                "(u8)",
        };
      }

      *value = v;
      return {};
    }

    if constexpr (sizeof(size_t) == sizeof(u16)) {
      if (v > 0xFFFF) {
        return Error{
            .type = &ERROR_SERIALIZER_NOT_ENOUGH_SPACE,
            .detail =
                "Cannot deserialize size_t value into this platform's size_t "
                "(u16)",
        };
      }

      *value = v;
      return {};
    }

    if constexpr (sizeof(size_t) == sizeof(u32)) {
      if (v > 0xFFFFFFFF) {
        return Error{
            .type = &ERROR_SERIALIZER_NOT_ENOUGH_SPACE,
            .detail =
                "Cannot deserialize size_t value into this platform's size_t "
                "(u32)",
        };
      }

      *value = v;
      return {};
    }

    *value = v;
    return {};
  }
}
#endif

Optional<Error> serialize(Serialization *S, char *value) {
  return serializePrimitive<char>(S, value);
}

}  // namespace sn
