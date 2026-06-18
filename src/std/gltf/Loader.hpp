#pragma once

#include "std/Arena.h"
#include "std/Result.hpp"
#include "std/Slice.hpp"
#include "std/gltf/Descriptor.hpp"

struct JsonValue;

namespace sn::gltf {
struct Error {
  enum Kind {
    // Data source is too short to be a valid file
    FILE_TOO_SHORT,
    // Magic value is bad
    BAD_MAGIC,
    // Unsupported file version
    BAD_VERSION,
    // File is shorter than the length it claims it has
    LENGTH_MISMATCH,
    // A chunk's header or its length is invalid
    INVALID_CHUNK,
    // File is missing either the JSON or the BIN chunk
    MISSING_CHUNKS,
    // Failed to parse the JSON descriptor
    INVALID_JSON,
    // A buffer references an URL which is not supported
    URL_SOURCE_NOT_SUPPORTED,
    // A buffer has no byte length
    BUFFER_MISSING_BYTE_LENGTH,
    // A buffer has an invalid index
    BUFFER_INVALID_INDEX,
    BUFFER_TOO_LONG,
    BUFFER_VIEW_INVALID_REF,
    BUFFER_VIEW_INVALID_SIZE,
    IMAGE_HAS_NO_BUFFER_VIEW,
    IMAGE_HAS_NO_MIME_TYPE,
    ACCESSOR_INVALID_BUFFER_VIEW,
    ACCESSOR_INVALID_COMPONENT_TYPE,
    ACCESSOR_INVALID_COUNT,
    ACCESSOR_INVALID_TYPE,
    ACCESSOR_NONSCALAR_INDEX_BUFFER,
    MESH_MISSING_PRIMITIVES,

    TEXTURE_MISSING_SOURCE,
    TEXTURE_INVALID_SOURCE,
    TEXTURE_INVALID_SAMPLER,

    NODE_INVALID_MATRIX,
    NODE_INVALID_CHILDREN_ARRAY,
    NODE_INVALID_MESH_REF,

    SCENE_MISSING_NODES,
    SCENE_EMPTY_NODE_LIST,
    SCENE_INVALID_NODE,

    SCENE_INDEX_OUT_OF_RANGE,

    VEC3_NOT_AN_ARRAY,
    VEC3_BAD_LENGTH,
    VEC3_BAD_COMPONENT_TYPE,

    VEC4_NOT_AN_ARRAY,
    VEC4_BAD_LENGTH,
    VEC4_BAD_COMPONENT_TYPE,

    QUAT_NOT_AN_ARRAY,
    QUAT_BAD_LENGTH,
    QUAT_BAD_COMPONENT_TYPE,
    QUAT_OUT_OF_RANGE,
  };

  Kind kind;
};

Result<gltf::File, Error> parseGltf(Arena *arena,
                                    JsonValue *json,
                                    Slice<const u8> binary);
Result<gltf::File, Error> parseGlb(Arena *arena, Slice<const u8> contents);
}  // namespace sn::gltf
