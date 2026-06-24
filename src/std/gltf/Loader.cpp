#include "std/gltf/Loader.hpp"
#include "std/Arena.h"
#include "std/SliceUtils.hpp"
#include "std/gltf/Math.hpp"
#include "std/json/Parser.hpp"
#include "std/json/Utils.hpp"
#include "std/json/Value.hpp"
#include "std/log.h"

#define DEFINE_KEY(Name, Value) \
  static const Slice<const char> Name = sliceFromConstChar(Value)

namespace sn::gltf {
DEFINE_KEY(K_INTERPOLATION, "interpolation");
DEFINE_KEY(K_LINEAR, "LINEAR");
DEFINE_KEY(K_STEP, "STEP");
DEFINE_KEY(K_CUBICSPLINE, "CUBICSPLINE");
DEFINE_KEY(K_TRANSLATION, "translation");
DEFINE_KEY(K_ROTATION, "rotation");
DEFINE_KEY(K_SCALE, "scale");
DEFINE_KEY(K_WEIGHTS, "weights");
DEFINE_KEY(K_BLEND, "BLEND");
DEFINE_KEY(K_MASK, "MASK");
DEFINE_KEY(K_SCALAR, "SCALAR");
DEFINE_KEY(K_VEC2, "VEC2");
DEFINE_KEY(K_VEC3, "VEC3");
DEFINE_KEY(K_VEC4, "VEC4");
DEFINE_KEY(K_MAT2, "MAT2");
DEFINE_KEY(K_MAT3, "MAT3");
DEFINE_KEY(K_MAT4, "MAT4");
DEFINE_KEY(K_POINT, "point");
DEFINE_KEY(K_SPOT, "spot");
DEFINE_KEY(K_DIRECTIONAL, "directional");
DEFINE_KEY(K_URI, "uri");
DEFINE_KEY(K_BYTE_LENGTH, "byteLength");
DEFINE_KEY(K_BYTE_OFFSET, "byteOffset");
DEFINE_KEY(K_BYTE_STRIDE, "byteStride");
DEFINE_KEY(K_BUFFER, "buffer");
DEFINE_KEY(K_TARGET, "target");
DEFINE_KEY(K_INDEX, "index");
DEFINE_KEY(K_TEXCOORD, "texcoord");
DEFINE_KEY(K_EMISSIVE_TEXTURE, "emissiveTexture");
DEFINE_KEY(K_MATERIAL, "material");
DEFINE_KEY(K_POSITION, "POSITION");
DEFINE_KEY(K_NORMAL, "NORMAL");
DEFINE_KEY(K_TANGENT, "TANGENT");
DEFINE_KEY(K_TEXCOORD_0, "TEXCOORD_0");
DEFINE_KEY(K_TEXCOORD_1, "TEXCOORD_1");
DEFINE_KEY(K_COLOR_0, "COLOR_0");
DEFINE_KEY(K_JOINTS_0, "JOINTS_0");
DEFINE_KEY(K_WEIGHTS_0, "WEIGHTS_0");
DEFINE_KEY(K_INDICES, "indices");
DEFINE_KEY(K_ATTRIBUTES, "attributes");
DEFINE_KEY(K_PRIMITIVES, "primitives");
DEFINE_KEY(K_MATRIX, "matrix");
DEFINE_KEY(K_CHILDREN, "children");
DEFINE_KEY(K_MESH, "mesh");
DEFINE_KEY(K_SKIN, "skin");
DEFINE_KEY(K_EXTENSIONS, "extensions");
DEFINE_KEY(K_KHR_LIGHTS_PUNCTUAL, "KHR_lights_punctual");
DEFINE_KEY(K_LIGHT, "light");
DEFINE_KEY(K_SAMPLER, "sampler");
DEFINE_KEY(K_SAMPLERS, "samplers");
DEFINE_KEY(K_CHANNELS, "channels");
DEFINE_KEY(K_NAME, "name");
DEFINE_KEY(K_INPUT, "input");
DEFINE_KEY(K_OUTPUT, "output");
DEFINE_KEY(K_NODE, "node");
DEFINE_KEY(K_NODES, "nodes");
DEFINE_KEY(K_PATH, "path");
DEFINE_KEY(K_PBR_METALLIC_ROUGHNESS, "pbrMetallicRoughness");
DEFINE_KEY(K_BASE_COLOR_TEXTURE, "baseColorTexture");
DEFINE_KEY(K_METALLIC_ROUGHNESS_TEXTURE, "metallicRoughnessTexture");
DEFINE_KEY(K_BASE_COLOR_FACTOR, "baseColorFactor");
DEFINE_KEY(K_METALLIC_FACTOR, "metallicFactor");
DEFINE_KEY(K_EMISSIVE_FACTOR, "emissiveFactor");
DEFINE_KEY(K_ROUGHNESS_FACTOR, "roughnessFactor");
DEFINE_KEY(K_MAG_FILTER, "magFilter");
DEFINE_KEY(K_MIN_FILTER, "minFilter");
DEFINE_KEY(K_WRAP_S, "wrapS");
DEFINE_KEY(K_WRAP_T, "wrapT");
DEFINE_KEY(K_JOINTS, "joints");
DEFINE_KEY(K_SOURCE, "source");
DEFINE_KEY(K_BUFFER_VIEW, "bufferView");
DEFINE_KEY(K_COMPONENT_TYPE, "componentType");
DEFINE_KEY(K_COUNT, "count");
DEFINE_KEY(K_TYPE, "type");
DEFINE_KEY(K_MIME_TYPE, "mimeType");
DEFINE_KEY(K_NORMAL_TEXTURE, "normalTexture");
DEFINE_KEY(K_ALPHA_CUTOFF, "alphaCutoff");
DEFINE_KEY(K_ALPHA_MODE, "alphaMode");
DEFINE_KEY(K_BUFFERS, "buffers");
DEFINE_KEY(K_IMAGES, "images");
DEFINE_KEY(K_BUFFERVIEWS, "bufferViews");
DEFINE_KEY(K_ACCESSORS, "accessors");
DEFINE_KEY(K_TEXTURES, "textures");
DEFINE_KEY(K_MESHES, "meshes");
DEFINE_KEY(K_MATERIALS, "materials");
DEFINE_KEY(K_SCENES, "scenes");
DEFINE_KEY(K_SCENE, "scene");
DEFINE_KEY(K_SKINS, "skins");
DEFINE_KEY(K_ANIMATIONS, "animations");
DEFINE_KEY(K_SKELETON, "skeleton");
DEFINE_KEY(K_INVERSE_BIND_MATRICES, "inverseBindMatrices");
DEFINE_KEY(K_COLOR, "color");
DEFINE_KEY(K_INTENSITY, "intensity");
DEFINE_KEY(K_RANGE, "range");
DEFINE_KEY(K_LIGHTS, "lights");

/**
 * \brief Load a 32-bit value stored in little endian order with zero-extension.
 */
static inline u32 loadLE_ZX(const u32 *src) {
  const u8 *src8 = reinterpret_cast<const u8 *>(src);
  return static_cast<u32>((src8[3] << 24) | (src8[2] << 16) | (src8[1] << 8) |
                          (src8[0] << 0));
}

/**
 * \brief Load a 32-bit value stored in little endian order with sign-extension.
 */
static inline i32 loadLE_SX(const i32 *src) {
  const u8 *src8 = reinterpret_cast<const u8 *>(src);
  return static_cast<i32>((src8[3] << 24) | (src8[2] << 16) | (src8[1] << 8) |
                          (src8[0] << 0));
}

struct ParseState {
  Slice<const u8> chunkBinary;
  File *file;
};

Result<Optional<Vec3>, Error> parseVec3(JsonValue *json) {
  if (json == nullptr) {
    return Optional<Vec3>();
  }

  if (!isArray(json)) {
    return Error{Error::VEC3_NOT_AN_ARRAY};
  }

  Slice<JsonValue> elems = json->array();
  if (elems.length != 3) {
    return Error{Error::VEC3_BAD_LENGTH};
  }

  Vec3 ret = {};

  for (auto [v, i] : elems) {
    if (!isNumeric(v)) {
      return Error{Error::VEC3_BAD_COMPONENT_TYPE};
    }

    ret.v[i] = asNumeric<f32>(v);
  }

  return Optional<Vec3>(ret);
}

Result<Optional<Quat>, Error> parseQuat(JsonValue *json) {
  if (json == nullptr) {
    return Optional<Quat>();
  }

  if (!isArray(json)) {
    return Error{Error::QUAT_NOT_AN_ARRAY};
  }

  Slice<JsonValue> elems = json->array();
  if (elems.length != 4) {
    return Error{Error::QUAT_BAD_LENGTH};
  }

  Quat ret = {};

  // NOTE(danielm): order of components match in both glTF and GLM: XYZW
  for (auto [v, i] : elems) {
    if (!isNumeric(v)) {
      return Error{Error::QUAT_BAD_COMPONENT_TYPE};
    }

    f32 x = asNumeric<f32>(v);
    if (x < -1.0f || 1.0f < x) {
      return Error{Error::QUAT_OUT_OF_RANGE};
    }
    ret.v[i] = x;
  }

  return Optional<Quat>(ret);
}

Result<Optional<Vec4>, Error> parseVec4(JsonValue *json) {
  if (json == nullptr) {
    return Optional<Vec4>();
  }

  if (!isArray(json)) {
    return Error{Error::VEC4_NOT_AN_ARRAY};
  }

  Slice<JsonValue> elems = json->array();
  if (elems.length != 4) {
    return Error{Error::VEC4_BAD_LENGTH};
  }

  Vec4 ret = {};

  for (auto [v, i] : elems) {
    if (!isNumeric(v)) {
      return Error{Error::VEC4_BAD_COMPONENT_TYPE};
    }

    ret.v[i] = asNumeric<f32>(v);
  }

  return Optional<Vec4>(ret);
}

/**
 * \brief Parse glTF entities from a given JSON array.
 * \param arena Allocator
 * \param state Parse state
 * \param array JSON array
 * \param parseEntity Callable that parses an element of the JSON array into an
 * object and returns `Result<T, Error>`.
 * \param out When non-NULL, the allocated entity array is written to the
 * provided address.
 */
template <typename T, typename C>
Result<Slice<T>, Error> parseEntities(Arena *arena,
                                      ParseState &state,
                                      JsonValue *array,
                                      C &&parseEntity,
                                      Slice<T> *out = nullptr) {
  if (array == nullptr) {
    return Slice<T>{nullptr, 0};
  }
  DCHECK(isArray(array));

  Slice<JsonValue> elems = array->array();
  if (elems.empty()) {
    return Slice<T>{nullptr, 0};
  }

  Slice<T> entities;
  alloc(arena, elems.length, entities);

  if (out != nullptr) {
    *out = entities;
  }

  for (auto [elem, idxElem] : elems) {
    Result<T, Error> res = parseEntity(arena, state, idxElem, &elem);
    if (res.isErr()) {
      return res.unwrapErr();
    }

    entities[idxElem] = res.unwrap();
  }

  return entities;
}

Result<Buffer, Error> parseBuffer(Arena *arena,
                                  ParseState &state,
                                  u32 idx,
                                  JsonValue *json) {
  JsonValue *byteLength =
      getKeyValueOfType(json, K_BYTE_LENGTH, JsonType::Number);
  JsonValue *uri = getKeyValueOfType(json, K_URI, JsonType::String);

  if (uri != nullptr) {
    return Error{Error::URL_SOURCE_NOT_SUPPORTED};
  }

  if (byteLength == nullptr) {
    return Error{Error::BUFFER_MISSING_BYTE_LENGTH};
  }

  u32 byteLength_v = asNumeric<u32>(byteLength);
  if (idx != 0) {
    return Error{Error::BUFFER_INVALID_INDEX};
  }

  Buffer ret = {};
  ret.data = state.chunkBinary.subarray(0, byteLength_v);
  return ret;
}

template <typename T>
Optional<T> tryGetNumericProp(JsonValue *obj, Slice<const char> key) {
  JsonValue *v = getKeyValueOfType(obj, key, JsonType::Number);
  if (v == nullptr) {
    return {};
  }

  return asNumeric<T>(v);
}

Result<BufferView, Error> parseBufferView(Arena *arena,
                                          ParseState &state,
                                          u32 idx,
                                          JsonValue *json) {
  JsonValue *buffer = getKeyValueOfType(json, K_BUFFER, JsonType::Number);
  JsonValue *byteLength =
      getKeyValueOfType(json, K_BYTE_LENGTH, JsonType::Number);

  if (buffer == nullptr) {
    return Error{Error::BUFFER_VIEW_INVALID_REF};
  }

  if (byteLength == nullptr) {
    return Error{Error::BUFFER_VIEW_INVALID_SIZE};
  }

  u32 idxBuffer = asNumeric<u32>(buffer);

  BufferView ret = {};
  ret.buffer = &state.file->buffers[idxBuffer];
  ret.byteLength = asNumeric<u32>(byteLength);
  ret.byteOffset = tryGetNumericProp<u32>(json, K_BYTE_OFFSET).valueOr(0);
  ret.byteStride = tryGetNumericProp<u32>(json, K_BYTE_STRIDE).valueOr(0);
  ret.target = tryGetNumericProp<u32>(json, K_TARGET).valueOr(0);
  return ret;
}

Result<Image, Error> parseImage(Arena *arena,
                                ParseState &state,
                                u32 idx,
                                JsonValue *json) {
  JsonValue *uri = getKeyValueOfType(json, K_URI, JsonType::String);
  if (uri != nullptr) {
    return Error{Error::URL_SOURCE_NOT_SUPPORTED};
  }

  JsonValue *bufferView =
      getKeyValueOfType(json, K_BUFFER_VIEW, JsonType::Number);
  if (bufferView == nullptr) {
    return Error{Error::IMAGE_HAS_NO_BUFFER_VIEW};
  }

  JsonValue *mimeType = getKeyValueOfType(json, K_MIME_TYPE, JsonType::String);
  if (mimeType == nullptr) {
    return Error{Error::IMAGE_HAS_NO_MIME_TYPE};
  }

  Image ret = {};
  ret.idxSelf = idx;
  ret.bufferView = &state.file->bufferViews[asNumeric<u32>(bufferView)];
  ret.mimeType = duplicate(arena, mimeType->string());
  return ret;
}

Result<Accessor, Error> parseAccessor(Arena *arena,
                                      ParseState &state,
                                      u32 idx,
                                      JsonValue *json) {
  JsonValue *bufferView =
      getKeyValueOfType(json, K_BUFFER_VIEW, JsonType::Number);
  JsonValue *componentType =
      getKeyValueOfType(json, K_COMPONENT_TYPE, JsonType::Number);
  JsonValue *count = getKeyValueOfType(json, K_COUNT, JsonType::Number);
  JsonValue *type = getKeyValueOfType(json, K_TYPE, JsonType::String);
  JsonValue *byteOffset =
      getKeyValueOfType(json, K_BYTE_OFFSET, JsonType::Number);

  if (bufferView == nullptr) {
    // FIXME(danielm): we must accept handle this case by creating a
    // zero-initialized buffer for this
    return Error{Error::ACCESSOR_INVALID_BUFFER_VIEW};
  }

  if (componentType == nullptr) {
    return Error{Error::ACCESSOR_INVALID_COMPONENT_TYPE};
  }

  if (count == nullptr) {
    return Error{Error::ACCESSOR_INVALID_COUNT};
  }

  if (type == nullptr) {
    return Error{Error::ACCESSOR_INVALID_TYPE};
  }

  Accessor accessor = {};
  accessor.bufferView = &state.file->bufferViews[asNumeric<u32>(bufferView)];
  accessor.componentType = asNumeric<u32>(componentType);
  accessor.count = asNumeric<u32>(count);
  accessor.byteOffset = tryGetNumericProp<u64>(json, K_BYTE_OFFSET).valueOr(0);

  Slice<const char> typeStr = type->string();
  if (compareAsString(typeStr, K_SCALAR)) {
    accessor.type = AccessorType::SCALAR;
  } else if (compareAsString(typeStr, K_VEC2)) {
    accessor.type = AccessorType::VEC2;
  } else if (compareAsString(typeStr, K_VEC3)) {
    accessor.type = AccessorType::VEC3;
  } else if (compareAsString(typeStr, K_VEC4)) {
    accessor.type = AccessorType::VEC4;
  } else if (compareAsString(typeStr, K_MAT2)) {
    accessor.type = AccessorType::MAT2;
  } else if (compareAsString(typeStr, K_MAT3)) {
    accessor.type = AccessorType::MAT3;
  } else if (compareAsString(typeStr, K_MAT4)) {
    accessor.type = AccessorType::MAT4;
  } else {
    return Error{Error::ACCESSOR_INVALID_TYPE};
  }

  return accessor;
}

Result<TextureInfo, Error> parseTextureInfo(Arena *arena,
                                            ParseState &state,
                                            JsonValue *json) {
  JsonValue *index = getKeyValueOfType(json, K_INDEX, JsonType::Number);
  JsonValue *texCoord = getKeyValueOfType(json, K_TEXCOORD, JsonType::Number);

  TextureInfo ret = {
      .texture = nullptr,
      .texcoord = 0,
  };

  if (index) {
    u32 idxTex = asNumeric<u32>(index);
    ret.texture = &state.file->textures[idxTex];
  }

  if (texCoord) {
    ret.texcoord = asNumeric<u16>(texCoord);
  }

  return ret;
}

Result<MetallicRoughness, Error> parseMetallicRoughness(Arena *arena,
                                                        ParseState &state,
                                                        JsonValue *json) {
  MetallicRoughness ret = {};

  auto bcf =
      parseVec4(getKeyValueOfType(json, K_BASE_COLOR_FACTOR, JsonType::Array));
  if (bcf.isErr()) {
    return bcf.unwrapErr();
  }
  ret.baseColorFactor = bcf->valueOr(Vec4(1, 1, 1, 1));

  JsonValue *baseColorTexture =
      getKeyValueOfType(json, K_BASE_COLOR_TEXTURE, JsonType::Object);
  if (baseColorTexture != nullptr) {
    auto bct = parseTextureInfo(arena, state, baseColorTexture);
    if (bct.isErr()) {
      return bct.unwrapErr();
    }

    ret.baseColorMap = bct.unwrap();
  }

  ret.metallicFactor =
      tryGetNumericProp<f32>(json, K_METALLIC_FACTOR).valueOr(0.0f);
  ret.roughnessFactor =
      tryGetNumericProp<f32>(json, K_ROUGHNESS_FACTOR).valueOr(1.0f);

  JsonValue *metallicRoughnessTexture =
      getKeyValueOfType(json, K_METALLIC_ROUGHNESS_TEXTURE, JsonType::Object);
  if (metallicRoughnessTexture != nullptr) {
    auto mrm = parseTextureInfo(arena, state, metallicRoughnessTexture);
    if (mrm.isErr()) {
      return mrm.unwrapErr();
    }

    ret.metallicRoughnessMap = mrm.unwrap();
  }

  return ret;
}

Result<Material, Error> parseMaterial(Arena *arena,
                                      ParseState &state,
                                      u32 idx,
                                      JsonValue *json) {
  Material mat = {};
  mat.idxSelf = idx;

  JsonValue *metallicRoughness =
      getKeyValueOfType(json, K_PBR_METALLIC_ROUGHNESS, JsonType::Object);
  if (metallicRoughness != nullptr) {
    auto res = parseMetallicRoughness(arena, state, metallicRoughness);
    if (res.isErr()) {
      return res.unwrapErr();
    }

    mat.metallicRoughness = res.unwrap();
  }

  // TODO(danielm): implement the rest
  mat.alphaMode = AlphaMode::OPAQUE;
  return mat;
}

template <typename T>
static T *tryGetEntityByIndex(Slice<T> arr, JsonValue *index) {
  if (index == nullptr) {
    return nullptr;
  }

  u32 i = asNumeric<u32>(index);

  if (arr.length <= i) {
    return nullptr;
  }

  return &arr[i];
}

template <typename T>
static T *tryGetEntityByIndex(Slice<T> arr,
                              JsonValue *attributes,
                              Slice<const char> key) {
  if (attributes == nullptr) {
    return nullptr;
  }

  JsonValue *ref = getKeyValueOfType(attributes, key, JsonType::Number);
  return tryGetEntityByIndex<T>(arr, ref);
}

static Material *tryGetMaterialByIndex(const ParseState &state,
                                       JsonValue *attributes,
                                       Slice<const char> key) {
  return tryGetEntityByIndex(state.file->materials, attributes, key);
}

static Accessor *tryGetAccessorByIndex(const ParseState &state,
                                       JsonValue *attributes,
                                       Slice<const char> key) {
  return tryGetEntityByIndex(state.file->accessors, attributes, key);
}

Result<Primitive, Error> parsePrimitive(Arena *arena,
                                        ParseState &state,
                                        u32 idx,
                                        JsonValue *json) {
  JsonValue *material = getKeyValueOfType(json, K_MATERIAL, JsonType::Number);
  JsonValue *indices = getKeyValueOfType(json, K_INDICES, JsonType::Number);
  JsonValue *attributes =
      getKeyValueOfType(json, K_ATTRIBUTES, JsonType::Object);

  Primitive prim = {};

  prim.material = tryGetMaterialByIndex(state, json, K_MATERIAL);
  prim.indices = tryGetAccessorByIndex(state, json, K_INDICES);
  if (prim.indices->type != AccessorType::SCALAR) {
    return Error{Error::ACCESSOR_NONSCALAR_INDEX_BUFFER};
  }

  prim.position = tryGetAccessorByIndex(state, attributes, K_POSITION);
  prim.normal = tryGetAccessorByIndex(state, attributes, K_NORMAL);
  prim.tangent = tryGetAccessorByIndex(state, attributes, K_TANGENT);
  prim.texcoord0 = tryGetAccessorByIndex(state, attributes, K_TEXCOORD_0);
  prim.texcoord1 = tryGetAccessorByIndex(state, attributes, K_TEXCOORD_1);
  prim.color0 = tryGetAccessorByIndex(state, attributes, K_COLOR_0);
  prim.joints0 = tryGetAccessorByIndex(state, attributes, K_JOINTS_0);
  prim.weights0 = tryGetAccessorByIndex(state, attributes, K_WEIGHTS_0);

  return prim;
}

Result<Mesh, Error> parseMesh(Arena *arena,
                              ParseState &state,
                              u32 idx,
                              JsonValue *json) {
  JsonValue *primitives =
      getKeyValueOfType(json, K_PRIMITIVES, JsonType::Array);
  if (primitives == nullptr) {
    return Error{Error::MESH_MISSING_PRIMITIVES};
  }

  auto resPrimitives =
      parseEntities<Primitive>(arena, state, primitives, parsePrimitive);
  if (resPrimitives.isErr()) {
    return resPrimitives.unwrapErr();
  }

  Mesh mesh = {
      .primitives = resPrimitives.unwrap(),
      .idxSelf = idx,
  };

  return mesh;
}

Result<Sampler, Error> parseSampler(Arena *arena,
                                    ParseState &state,
                                    u32 idx,
                                    JsonValue *json) {
  Sampler ret = {};

  ret.magFilter = tryGetNumericProp<u16>(json, K_MAG_FILTER).valueOr(GL_LINEAR);
  ret.minFilter = tryGetNumericProp<u16>(json, K_MIN_FILTER)
                      .valueOr(GL_NEAREST_MIPMAP_LINEAR);

  ret.wrapS = tryGetNumericProp<u16>(json, K_WRAP_S).valueOr(GL_REPEAT);
  ret.wrapT = tryGetNumericProp<u16>(json, K_WRAP_T).valueOr(GL_REPEAT);

  return ret;
}

Result<Texture, Error> parseTexture(Arena *arena,
                                    ParseState &state,
                                    u32 idx,
                                    JsonValue *json) {
  JsonValue *image = getKeyValueOfType(json, K_SOURCE, JsonType::Number);
  JsonValue *sampler = getKeyValueOfType(json, K_SAMPLER, JsonType::Number);
  if (image == nullptr) {
    return Error{Error::TEXTURE_MISSING_SOURCE};
  }

  Texture ret = {};
  ret.source = tryGetEntityByIndex(state.file->images, image);
  if (ret.source == nullptr) {
    return Error{Error::TEXTURE_INVALID_SOURCE};
  }

  if (sampler != nullptr) {
    ret.sampler = tryGetEntityByIndex(state.file->samplers, sampler);
    if (ret.sampler == nullptr) {
      return Error{Error::TEXTURE_INVALID_SAMPLER};
    }
  }

  return ret;
}

Result<Node, Error> parseNode(Arena *arena,
                              ParseState &state,
                              u32 idx,
                              JsonValue *json) {
  Node ret = {};

  JsonValue *matrix = getKeyValueOfType(json, K_MATRIX, JsonType::Array);
  JsonValue *children = getKeyValueOfType(json, K_CHILDREN, JsonType::Array);
  JsonValue *mesh = getKeyValueOfType(json, K_MESH, JsonType::Number);

  // Transform

  if (matrix != nullptr) {
    Slice<JsonValue> elems = matrix->array();
    if (elems.length != 16) {
      return Error{Error::NODE_INVALID_MATRIX};
    }

    f32 matrixComponents[16];

    for (u32 i = 0; i < 16; i++) {
      if (!isNumeric(elems[i])) {
        return Error{Error::NODE_INVALID_MATRIX};
      }

      matrixComponents[i] = asNumeric<f32>(elems[i]);
    }

    decompose(matrixComponents, &ret.localPosition, &ret.localOrientation,
              &ret.localScale);
  } else {
    Result<Optional<Vec3>, Error> translation =
        parseVec3(getKeyValueOfType(json, K_TRANSLATION, JsonType::Array));
    Result<Optional<Quat>, Error> rotation =
        parseQuat(getKeyValueOfType(json, K_ROTATION, JsonType::Array));
    Result<Optional<Vec3>, Error> scale =
        parseVec3(getKeyValueOfType(json, K_SCALE, JsonType::Array));

    if (translation.isErr()) {
      return translation.unwrapErr();
    }
    if (rotation.isErr()) {
      return rotation.unwrapErr();
    }
    if (scale.isErr()) {
      return scale.unwrapErr();
    }

    ret.localPosition = translation->valueOr(Vec3(0, 0, 0));
    ret.localOrientation = rotation->valueOr(Quat(0, 0, 0, 1));
    ret.localScale = scale->valueOr(Vec3(1, 1, 1));
  }

  ret.idxSelf = idx;

  // Child nodes
  if (children != nullptr) {
    // state.file->nodes in particular will get assigned before node parsing
    // begins. While the content of the nodes is only guaranteed to be zero
    // initialized at this point, this lets us take pointers to other nodes. We
    // also initialize `parent` pointers here.
    DCHECK(!state.file->nodes.empty());
    Node *self = &state.file->nodes[idx];

    Slice<JsonValue> elems = children->array();
    Slice<Node *> children;
    alloc(arena, elems.length, children);

    for (auto [e, i] : elems) {
      if (!isNumeric(e)) {
        return Error{Error::NODE_INVALID_CHILDREN_ARRAY};
      }

      u32 idxNode = asNumeric<u32>(e);
      Node &child = state.file->nodes[idxNode];
      children[i] = &child;
      child.parent = self;
    }

    ret.children = children;
  }

  ret.parent = state.file->nodes[idx].parent;

  // Attached entities
  if (mesh != nullptr) {
    u32 idxMesh = asNumeric<u32>(mesh);
    if (idxMesh >= state.file->meshes.length) {
      return Error{Error::NODE_INVALID_MESH_REF};
    }

    ret.mesh = &state.file->meshes[idxMesh];
  }

  return ret;
}

Result<Scene, Error> parseScene(Arena *arena,
                                ParseState &state,
                                u32 idx,
                                JsonValue *json) {
  JsonValue *nodes = getKeyValueOfType(json, K_NODES, JsonType::Array);

  if (nodes == nullptr) {
    return Error{Error::SCENE_MISSING_NODES};
  }

  Slice<JsonValue> elems = nodes->array();
  if (elems.empty()) {
    return Error{Error::SCENE_EMPTY_NODE_LIST};
  }

  Slice<Node *> nodesArr;
  alloc(arena, elems.length, nodesArr);

  for (auto [e, i] : elems) {
    if (!isNumeric(e)) {
      return Error{Error::SCENE_INVALID_NODE};
    }

    u32 idxNode = asNumeric<u32>(e);
    if (idxNode >= state.file->nodes.length) {
      return Error{Error::SCENE_INVALID_NODE};
    }
    nodesArr[i] = &state.file->nodes[idxNode];
  }

  return Scene{
      .rootNodes = nodesArr,
  };
}

Result<gltf::File, Error> parseGltf(Arena *arena,
                                    JsonValue *json,
                                    Slice<const u8> binary) {
  JsonValue *buffers = getKeyValueOfType(json, K_BUFFERS, JsonType::Array);
  JsonValue *images = getKeyValueOfType(json, K_IMAGES, JsonType::Array);
  JsonValue *bufferViews =
      getKeyValueOfType(json, K_BUFFERVIEWS, JsonType::Array);
  JsonValue *accessors = getKeyValueOfType(json, K_ACCESSORS, JsonType::Array);
  JsonValue *samplers = getKeyValueOfType(json, K_SAMPLERS, JsonType::Array);
  JsonValue *textures = getKeyValueOfType(json, K_TEXTURES, JsonType::Array);
  JsonValue *meshes = getKeyValueOfType(json, K_MESHES, JsonType::Array);
  JsonValue *materials = getKeyValueOfType(json, K_MATERIALS, JsonType::Array);
  JsonValue *nodes = getKeyValueOfType(json, K_NODES, JsonType::Array);
  JsonValue *scenes = getKeyValueOfType(json, K_SCENES, JsonType::Array);
  JsonValue *scene = getKeyValueOfType(json, K_SCENE, JsonType::Number);
  // JsonValue *extensions = getKeyValueOfType(json, K_EXTENSIONS,
  // JsonType::Object); JsonValue *skins = getKeyValueOfType(json, K_SKINS,
  // JsonType::Array); JsonValue *animations = getKeyValueOfType(json,
  // K_ANIMATIONS, JsonType::Array);

  File file = {};
  ParseState state = {
      .chunkBinary = binary,
      .file = &file,
  };

  auto resBuffers = parseEntities<Buffer>(arena, state, buffers, parseBuffer);
  if (resBuffers.isErr()) {
    return resBuffers.unwrapErr();
  }
  file.buffers = resBuffers.unwrap();

  auto resBufferViews =
      parseEntities<BufferView>(arena, state, bufferViews, parseBufferView);
  if (resBufferViews.isErr()) {
    return resBufferViews.unwrapErr();
  }
  file.bufferViews = resBufferViews.unwrap();

  auto resAccessors =
      parseEntities<Accessor>(arena, state, accessors, parseAccessor);
  if (resAccessors.isErr()) {
    return resAccessors.unwrapErr();
  }
  file.accessors = resAccessors.unwrap();

  auto resImages = parseEntities<Image>(arena, state, images, parseImage);
  if (resImages.isErr()) {
    return resImages.unwrapErr();
  }
  file.images = resImages.unwrap();

  auto resSamplers =
      parseEntities<Sampler>(arena, state, samplers, parseSampler);
  if (resSamplers.isErr()) {
    return resSamplers.unwrapErr();
  }
  file.samplers = resSamplers.unwrap();

  auto resTextures =
      parseEntities<Texture>(arena, state, textures, parseTexture);
  if (resTextures.isErr()) {
    return resTextures.unwrapErr();
  }
  file.textures = resTextures.unwrap();

  auto resMaterials =
      parseEntities<Material>(arena, state, materials, parseMaterial);
  if (resMaterials.isErr()) {
    return resMaterials.unwrapErr();
  }
  file.materials = resMaterials.unwrap();

  auto resMeshes = parseEntities<Mesh>(arena, state, meshes, parseMesh);
  if (resMeshes.isErr()) {
    return resMeshes.unwrapErr();
  }
  file.meshes = resMeshes.unwrap();

  auto resNodes =
      parseEntities<Node>(arena, state, nodes, parseNode, &file.nodes);
  if (resNodes.isErr()) {
    return resNodes.unwrapErr();
  }
  file.nodes = resNodes.unwrap();

  auto resScenes = parseEntities<Scene>(arena, state, scenes, parseScene);
  if (resScenes.isErr()) {
    return resScenes.unwrapErr();
  }
  file.scenes = resScenes.unwrap();

  if (scene != nullptr && isNumeric(scene)) {
    u32 idxScene = asNumeric<u32>(scene);
    if (idxScene >= file.scenes.length) {
      return Error{Error::SCENE_INDEX_OUT_OF_RANGE};
    }

    file.scene = &file.scenes[idxScene];
  }

  return file;
}

Result<gltf::File, Error> parseGlb(Arena *arena, Slice<const u8> contents) {
  if (contents.byteLength() < sizeof(glb::Header)) {
    return Error{Error::FILE_TOO_SHORT};
  }

  glb::Header header =
      contents.subarray(0, sizeof(glb::Header)).cast<const glb::Header>()[0];

  if (loadLE_ZX(&header.magic) != 0x46546C67) {
    return Error{Error::BAD_MAGIC};
  }

  if (loadLE_ZX(&header.version) != 2) {
    return Error{Error::BAD_VERSION};
  }

  const u32 totalLength = loadLE_ZX(&header.length);
  if (totalLength < contents.length) {
    return Error{Error::LENGTH_MISMATCH};
  }

  Slice<const u8> chunkJson;
  Slice<const u8> chunkBinary;

  Slice<const u8> chunks = contents.subarray(sizeof(glb::Header));
  while (!chunks.empty()) {
    auto chunkHeader = chunks.subarray(0, sizeof(glb::ChunkHeader));

    if (chunkHeader.length != sizeof(glb::ChunkHeader)) {
      return Error{Error::INVALID_CHUNK};
    }

    glb::ChunkHeader header = chunkHeader.cast<const glb::ChunkHeader>()[0];

    u32 type = loadLE_ZX(&header.type);
    u32 length = loadLE_ZX(&header.length);

    auto data = chunks.subarray(sizeof(glb::ChunkHeader),
                                sizeof(glb::ChunkHeader) + length);
    if (data.length != length) {
      return Error{Error::INVALID_CHUNK};
    }

    switch (type) {
      case 0x4E4F534A:
        chunkJson = data;
        break;
      case 0x004E4942:
        chunkBinary = data;
        break;
      default:
        log_warn("Unexpected GLB chunk with type 0x%x", type);
        break;
    }

    chunks = chunks.subarray(sizeof(glb::ChunkHeader) + length);
  }

  if (chunkJson.empty() || chunkBinary.empty()) {
    return Error{Error::MISSING_CHUNKS};
  }

  Arena::Scope temp = getScratch(&arena, 1);

  Slice<const char> json = chunkJson.cast<const char>();
  JsonValue root = {};
  if (!tryParseValue(temp, json, root)) {
    return Error{Error::INVALID_JSON};
  }

  return parseGltf(arena, &root, chunkBinary);
}
}  // namespace sn::gltf
