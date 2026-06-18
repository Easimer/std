#pragma once

#include "std/Optional.hpp"
#include "std/Slice.hpp"
#include "std/Types.h"

namespace sn::gltf {
enum {
  GL_BYTE = 5120,
  GL_UNSIGNED_BYTE = 5121,
  GL_SHORT = 5122,
  GL_UNSIGNED_SHORT = 5123,
  GL_INT = 5124,
  GL_UNSIGNED_INT = 5125,
  GL_FLOAT = 5126,
  GL_NEAREST = 9728,
  GL_LINEAR = 9729,
  GL_NEAREST_MIPMAP_NEAREST = 9984,
  GL_LINEAR_MIPMAP_NEAREST = 9985,
  GL_NEAREST_MIPMAP_LINEAR = 9986,
  GL_LINEAR_MIPMAP_LINEAR = 9987,
  GL_CLAMP_TO_EDGE = 33071,
  GL_MIRRORED_REPEAT = 33648,
  GL_REPEAT = 10497,
  GL_ARRAY_BUFFER = 34962,
  GL_ELEMENT_ARRAY_BUFFER = 34963,
};

enum class AccessorType : u8 {
  SCALAR = 0,
  VEC2,
  VEC3,
  VEC4,
  MAT2,
  MAT3,
  MAT4,
};

enum class InterpolationKind : u8 {
  LINEAR = 0,
  STEP,
  CUBICSPLINE,
};

enum class AnimationChannelPath : u8 {
  TRANSLATION = 0,
  ROTATION,
  SCALE,
  WEIGHTS,
};

enum class AlphaMode : u16 {
  OPAQUE = 0,
  MASK_0 = 0,
  MASK_255 = 255,
  BLEND,
};

enum class PunctualLightKind : u8 {
  POINT = 0,
  SPOT,
  DIRECTIONAL,
};

namespace glb {
struct Header {
  u32 magic;
  u32 version;
  u32 length;
};

struct ChunkHeader {
  u32 length;
  u32 type;
  // data
};

struct Chunk {
  Slice<const u8> data;
};
}  // namespace glb

struct Vec3;
struct Vec4;
struct Quat;

struct Buffer;
struct BufferView;
struct Accessor;
struct Image;
struct Sampler;
struct Texture;
struct TextureInfo;
struct NormalTextureInfo;
struct MetallicRoughness;
struct Material;
struct Primitive;
struct Mesh;
struct Camera;
struct PunctualLightSpot;
struct PunctualLight;
struct Skin;
struct Node;
struct AnimationSampler;
struct AnimationChannel;
struct Animation;
struct Scene;
struct File;

struct Vec3 {
  union {
    struct {
      f32 x;
      f32 y;
      f32 z;
    };

    struct {
      f32 r;
      f32 g;
      f32 b;
    };

    f32 v[3];
  };
};

struct Vec4 {
  union {
    struct {
      f32 x;
      f32 y;
      f32 z;
      f32 w;
    };
    f32 v[4];
  };
};

struct Quat {
  union {
    struct {
      f32 x;
      f32 y;
      f32 z;
      f32 w;
    };
    f32 v[4];
  };
  };

struct Buffer {
  Slice<const u8> data;
};

struct BufferView {
  Buffer *buffer = nullptr;
  u16 target = 0;
  u32 byteOffset = 0;
  u32 byteLength = 0;
  u32 byteStride = 0;

  Slice<const u8> slice() const {
    if (buffer == nullptr) {
      return {nullptr, 0};
    }

    return buffer->data.subarray(byteOffset, byteOffset + byteLength);
  }
};

struct Accessor {
  BufferView *bufferView = nullptr;

  u32 byteOffset = 0;
  u32 componentType = 0;
  u32 count = 0;

  AccessorType type = AccessorType::SCALAR;
  bool normalized = false;
};

struct Image {
  u32 idxSelf;
  BufferView *bufferView = nullptr;
  Slice<char> mimeType;
};

struct Sampler {
  u16 minFilter = GL_LINEAR;
  u16 magFilter = GL_LINEAR;
  u16 wrapS = GL_REPEAT;
  u16 wrapT = GL_REPEAT;
};

struct Texture {
  Image *source = nullptr;
  Sampler *sampler = nullptr;
};

struct TextureInfo {
  Texture *texture = nullptr;
  u16 texcoord = 0;
};

struct NormalTextureInfo {
  TextureInfo *textureInfo = nullptr;
  f32 scale = 1.0f;
};

struct MetallicRoughness {
  Vec4 baseColorFactor = {1, 1, 1, 1};
  f32 metallicFactor = 0.0f;
  f32 roughnessFactor = 1.0f;
  Optional<TextureInfo> baseColorMap;
  Optional<TextureInfo> metallicRoughnessMap;
};

struct Material {
  u32 idxSelf;
  Optional<MetallicRoughness> metallicRoughness;
  Optional<NormalTextureInfo> normalMap;
  Optional<TextureInfo> emissiveMap;
  Vec3 emissiveFactor = {0, 0, 0};
  AlphaMode alphaMode = AlphaMode::OPAQUE;
};

struct Primitive {
  Accessor *position = nullptr;
  Accessor *normal = nullptr;
  Accessor *tangent = nullptr;
  Accessor *texcoord0 = nullptr;
  Accessor *texcoord1 = nullptr;
  Accessor *color0 = nullptr;
  Accessor *joints0 = nullptr;
  Accessor *weights0 = nullptr;

  Accessor *indices = nullptr;

  Material *material = nullptr;
};

struct Mesh {
  Slice<Primitive> primitives;
  u32 idxSelf;
};

struct Camera {};

struct PunctualLightSpot {
  f32 innerConeAngle;
  f32 outerConeAngle;
};

struct PunctualLight {
  PunctualLightKind kind;
  Vec3 color;
  f32 intensity;
  f32 range;
  PunctualLightSpot *spot = nullptr;
};

struct Skin {
  Slice<Node *> joints;
  Node *skeleton = nullptr;
  Accessor *inverseBindMatrices = nullptr;
  u16 idxSelf;
};

struct Node {
  Vec3 localPosition = {0, 0, 0};
  Quat localOrientation = Quat(1, 0, 0, 0);
  Vec3 localScale = {1, 1, 1};

  u32 idxSelf;
  Node *parent = nullptr;
  Slice<Node *> children;

  Mesh *mesh = nullptr;
  Skin *skin = nullptr;
  Camera *camera = nullptr;
  PunctualLight *light = nullptr;
};

struct Scene {
  Slice<Node *> rootNodes;
};

struct File {
  Slice<Buffer> buffers;
  Slice<Image> images;
  Slice<BufferView> bufferViews;
  Slice<Accessor> accessors;
  Slice<Sampler> samplers;
  Slice<Texture> textures;
  Slice<Material> materials;
  Slice<Mesh> meshes;
  Slice<Node> nodes;
  Slice<Scene> scenes;
  Slice<PunctualLight> punctualLights;

  Scene *scene = nullptr;
};

}  // namespace sn::gltf
