/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Arena.h"
#include "std/Error.h"
#include "std/Optional.hpp"
#include "std/Slice.hpp"
#include "std/Types.h"

namespace sn {
struct ISerializerStream {
  [[nodiscard]] virtual Optional<Error> read(MutSlice<u8> buffer) noexcept = 0;
  [[nodiscard]] virtual Optional<Error> write(Slice<u8> buffer) noexcept = 0;
};

/** Serializer state. */
struct Serialization {
  /**
   * Stream to use.
   */
  ISerializerStream *stream = nullptr;
  /**
   * Arena where Slice<T> backing buffers will be allocated when deserializing.
   */
  Arena *arena = nullptr;
  /**
   * Version number of the format. Serializer functions can use this value to
   * decide which fields to skip. It's up to the caller who starts the
   * serialization to set this value.
   */
  u32 version = 0;
  /**
   * Serialization direction. Used by (usually) leaf serialization functions to
   * choose between reading or writing the stream.
   */
  bool isWriting = false;
};

extern const ErrorType ERROR_SERIALIZER_INVALID_OPERATION;
extern const ErrorType ERROR_SERIALIZER_PAST_END_OF_STREAM;
extern const ErrorType ERROR_SERIALIZER_NOT_ENOUGH_SPACE;

struct SerializeVU128Tag {};
struct SerializeNoallocTag {};
template <typename U>
struct SerializeEnumTag {};

struct FalseType {
  static constexpr bool value = false;
};

struct TrueType {
  static constexpr bool value = true;
};

template <typename T>
struct SerializationPrimitive : FalseType {};

#define SN_STD_SERIALIZATION_PRIMITIVE(T) \
  template <>                             \
  struct SerializationPrimitive<T> : TrueType {}

SN_STD_SERIALIZATION_PRIMITIVE(bool);

SN_STD_SERIALIZATION_PRIMITIVE(u8);
SN_STD_SERIALIZATION_PRIMITIVE(u16);
SN_STD_SERIALIZATION_PRIMITIVE(u32);
SN_STD_SERIALIZATION_PRIMITIVE(u64);

SN_STD_SERIALIZATION_PRIMITIVE(i8);
SN_STD_SERIALIZATION_PRIMITIVE(i16);
SN_STD_SERIALIZATION_PRIMITIVE(i32);
SN_STD_SERIALIZATION_PRIMITIVE(i64);

SN_STD_SERIALIZATION_PRIMITIVE(f32);
SN_STD_SERIALIZATION_PRIMITIVE(f64);

[[nodiscard]] Optional<Error> serializeBytes(Serialization *S,
                                             u8 *value,
                                             size_t count);

[[nodiscard]] Optional<Error> serialize(Serialization *S, bool *value);

[[nodiscard]] Optional<Error> serialize(Serialization *S, u8 *value);
[[nodiscard]] Optional<Error> serialize(Serialization *S, u16 *value);
[[nodiscard]] Optional<Error> serialize(Serialization *S, u32 *value);
[[nodiscard]] Optional<Error> serialize(Serialization *S, u64 *value);

[[nodiscard]] Optional<Error> serialize(Serialization *S, i8 *value);
[[nodiscard]] Optional<Error> serialize(Serialization *S, i16 *value);
[[nodiscard]] Optional<Error> serialize(Serialization *S, i32 *value);
[[nodiscard]] Optional<Error> serialize(Serialization *S, i64 *value);

[[nodiscard]] Optional<Error> serialize(Serialization *S, f32 *value);
[[nodiscard]] Optional<Error> serialize(Serialization *S, f64 *value);

/**
 * \brief Serializes a size_t into a u64. Deserialization may fail if the value
 * as a u64 cannot fit into this platform's size_t.
 */
[[nodiscard]] Optional<Error> serialize(Serialization *S, size_t *value);

[[nodiscard]] Optional<Error> serialize(Serialization *S, char *value);

[[nodiscard]] Optional<Error> serialize(Serialization *S,
                                        u32 *value,
                                        const SerializeVU128Tag &);

#define SN_TRY_SERIALIZE(Call)  \
  do {                          \
    Optional<Error> err = Call; \
    if (err) {                  \
      return err;               \
    }                           \
  } while (0)

#define SN_MUST_SERIALIZE(Call) \
  do {                          \
    Optional<Error> err = Call; \
    CHECK(!err);                \
  } while (0)

template <typename T>
[[nodiscard]] inline Optional<Error> serialize(Serialization *S,
                                               MutSlice<T> *value) {
  SN_TRY_SERIALIZE(serialize(S, &value->length));

  if (!S->isWriting) {
    DCHECK(S->arena != nullptr);
    value->data = (T *)alloc(S->arena, sizeof(T), alignof(T), value->length);
  }

  for (auto [elem, _] : *value) {
    SN_TRY_SERIALIZE(serialize(S, &elem));
  }

  return {};
}

template <typename T>
[[nodiscard]] inline Optional<Error> serialize(Serialization *S,
                                               MutSlice<T> *value,
                                               const SerializeNoallocTag &) {
  const size_t lenBuf = value->length;

  size_t numElems = lenBuf;
  SN_TRY_SERIALIZE(serialize(S, &numElems));

  if (!S->isWriting) {
    if (lenBuf < numElems) {
      return Error{.type = &ERROR_SERIALIZER_NOT_ENOUGH_SPACE};
    }
  }

  value->length = numElems;

  for (auto [elem, _] : *value) {
    SN_TRY_SERIALIZE(serialize(S, &elem));
  }

  return {};
}

template <>
[[nodiscard]] inline Optional<Error> serialize<char>(Serialization *S,
                                                     MutSlice<char> *value) {
  Optional<Error> err = serialize(S, &value->length);
  if (err) {
    return err;
  }

  if (!S->isWriting) {
    DCHECK(S->arena != nullptr);
    value->data =
        (char *)alloc(S->arena, sizeof(char), alignof(char), value->length);
  }

  return serializeBytes(S, (u8 *)value->data, value->length);
}

template <typename T>
[[nodiscard]] inline Optional<Error> serialize(Serialization *S,
                                               Slice<T> *value) {
  if (S->isWriting) {
    SN_TRY_SERIALIZE(serialize(S, &value->length));

    for (auto [elem, _] : *value) {
      T copy = elem;
      SN_TRY_SERIALIZE(serialize(S, &copy));
    }
  } else {
    MutSlice<T> temp;
    SN_TRY_SERIALIZE(serialize(S, &temp.length));
    DCHECK(S->arena != nullptr);
    temp.data = (T *)alloc(S->arena, sizeof(T), alignof(T), temp.length);
    for (auto [elem, _] : temp) {
      SN_TRY_SERIALIZE(serialize(S, &elem));
    }

    *value = temp;
  }
  return {};
}

template <typename T, size_t N>
[[nodiscard]] inline Optional<Error> serialize(Serialization *S, T (*p)[N]) {
  T *elements = *p;
  for (size_t i = 0; i < N; i++) {
    Optional<Error> err = serialize(S, &elements[i]);
    if (err) {
      return err;
    }
  }

  return {};
}

/**
 * \brief Serialize an enum value of type `T` into an integer of type `U`.
 */
template <typename U = i32, typename T>
[[nodiscard]] inline Optional<Error> serialize(Serialization *S,
                                               T *value,
                                               const SerializeEnumTag<U> &) {
  if (S->isWriting) {
    U c = (U)(*value);
    SN_TRY_SERIALIZE(serialize(S, &c));
  } else {
    U c;
    SN_TRY_SERIALIZE(serialize(S, &c));
    *value = (T)c;
  }

  return {};
}

}  // namespace sn
