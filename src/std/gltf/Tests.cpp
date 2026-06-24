#define _CRT_SECURE_NO_WARNINGS // fopen

#include "std/gltf/Descriptor.hpp"
#include "std/gltf/Loader.hpp"
#include "std/gltf/Math.hpp"
#include "std/SliceUtils.hpp"
#include "std/Testing.hpp"

#include <stdio.h>

// clang-format off
#define CHECK_VEC_CLOSE_ENOUGH(Actual, Expected, Epsilon)                \
  CHECK(dist_l1((Actual), (Expected)) < (Epsilon))
// clang-format on

using namespace sn::gltf;

static inline bool isQuatCloseEnough(Quat actual,
                                     Quat expected,
                                     f32 epsilon) {
  Quat delta = actual * conjugate(expected);
  return sn::gltf::abs(1.0f - delta.w) < epsilon;
}

#define CHECK_QUAT_CLOSE_ENOUGH(Actual, Expected, Epsilon) \
  CHECK(isQuatCloseEnough(Actual, Expected, Epsilon) ||    \
        isQuatCloseEnough(negate(Actual), Expected, Epsilon))

static sn::gltf::File parseModel(Arena *arena, Slice<const char> path) {
  Arena::Scope temp = getScratch(&arena, 1);
  Slice<const u8> src = snTestReadAsset(temp, path).asConst();
  Result<sn::gltf::File, sn::gltf::Error> res = sn::gltf::parseGlb(arena, src);
  CHECK(res.isOk());

  return res.unwrap();
}

static const Slice<const char> BOX_GLB = sliceFromConstChar("Box.glb");
static const Slice<const char> BOX_TEXTURED_GLB =
    sliceFromConstChar("BoxTextured.glb");

SN_TEST(Gltf, Box_buffers) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_GLB);

  CHECK(file.buffers.length == 1);
  CHECK(file.buffers[0].data.length == 648);
}

SN_TEST(Gltf, Box_bufferViews) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_GLB);

  CHECK(file.bufferViews.length == 2);

  auto *buffer0 = &file.buffers[0];

  CHECK(file.bufferViews[0].buffer == buffer0);
  CHECK(file.bufferViews[0].byteOffset == 576);
  CHECK(file.bufferViews[0].byteLength == 72);
  CHECK(file.bufferViews[0].byteStride == 0);
  CHECK(file.bufferViews[0].target == sn::gltf::GL_ELEMENT_ARRAY_BUFFER);

  CHECK(file.bufferViews[1].buffer == buffer0);
  CHECK(file.bufferViews[1].byteOffset == 0);
  CHECK(file.bufferViews[1].byteLength == 576);
  CHECK(file.bufferViews[1].byteStride == 12);
  CHECK(file.bufferViews[1].target == sn::gltf::GL_ARRAY_BUFFER);
}

SN_TEST(Gltf, Box_accessors) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_GLB);

  CHECK(file.accessors.length == 3);

  auto *bufferView0 = &file.bufferViews[0];
  auto *bufferView1 = &file.bufferViews[1];

  auto &accessor0 = file.accessors[0];
  auto &accessor1 = file.accessors[1];
  auto &accessor2 = file.accessors[2];

  CHECK(accessor0.bufferView == bufferView0);
  CHECK(accessor0.byteOffset == 0);
  CHECK(accessor0.componentType == sn::gltf::GL_UNSIGNED_SHORT);
  CHECK(accessor0.count == 36);
  CHECK(accessor0.type == sn::gltf::AccessorType::SCALAR);

  CHECK(accessor1.bufferView == bufferView1);
  CHECK(accessor1.byteOffset == 0);
  CHECK(accessor1.componentType == sn::gltf::GL_FLOAT);
  CHECK(accessor1.count == 24);
  CHECK(accessor1.type == sn::gltf::AccessorType::VEC3);

  CHECK(accessor2.bufferView == bufferView1);
  CHECK(accessor2.byteOffset == 288);
  CHECK(accessor2.componentType == sn::gltf::GL_FLOAT);
  CHECK(accessor2.count == 24);
  CHECK(accessor2.type == sn::gltf::AccessorType::VEC3);
}

SN_TEST(Gltf, Box_materials) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_GLB);

  CHECK(file.materials.length == 1);

  auto &mat0 = file.materials[0];

  CHECK(mat0.alphaMode == sn::gltf::AlphaMode::OPAQUE);
  CHECK(mat0.metallicRoughness.hasValue());

  CHECK_VEC_CLOSE_ENOUGH(mat0.metallicRoughness->baseColorFactor,
                         Vec4(0.8f, 0.0f, 0.0f, 1.0f), 0.01f);

  CHECK(fabsf(mat0.metallicRoughness->metallicFactor - 0.0f) < 0.01f);

  // Defaults
  CHECK(fabsf(mat0.metallicRoughness->roughnessFactor - 1.0f) < 0.01f);
  CHECK(!mat0.metallicRoughness->baseColorMap.hasValue());
  CHECK(!mat0.metallicRoughness->metallicRoughnessMap.hasValue());
  CHECK_VEC_CLOSE_ENOUGH(mat0.emissiveFactor, Vec3(0, 0, 0), 0.0001f);
  CHECK(!mat0.emissiveMap.hasValue());
}

SN_TEST(Gltf, Box_meshes) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_GLB);

  CHECK(file.meshes.length == 1);
  CHECK(file.materials.length == 1);
  CHECK(file.accessors.length == 3);

  auto &mat0 = file.materials[0];

  auto &mesh0 = file.meshes[0];
  CHECK(mesh0.primitives.length == 1);
  CHECK(mesh0.idxSelf == 0);

  auto &prim0 = mesh0.primitives[0];

  CHECK(prim0.material == &mat0);

  CHECK(prim0.position == &file.accessors[2]);
  CHECK(prim0.normal == &file.accessors[1]);
  CHECK(prim0.indices == &file.accessors[0]);
}

SN_TEST(Gltf, Box_nodes) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_GLB);

  CHECK(file.nodes.length == 2);
  CHECK(file.meshes.length > 0);

  auto &node0 = file.nodes[0];
  CHECK_VEC_CLOSE_ENOUGH(node0.localPosition, Vec3(0, 0, 0), 0.001f);
  CHECK_QUAT_CLOSE_ENOUGH(node0.localOrientation, Quat(-0.707f, 0, 0, 0.707f),
                          0.001f);
  CHECK_VEC_CLOSE_ENOUGH(node0.localScale, Vec3(1, 1, 1), 0.001f);
  CHECK(node0.idxSelf == 0);
  CHECK(node0.mesh == nullptr);
  CHECK(node0.camera == nullptr);
  CHECK(node0.light == nullptr);
  CHECK(node0.skin == nullptr);
  CHECK(node0.parent == nullptr);
  CHECK(node0.children.length == 1);
  CHECK(node0.children[0] == &file.nodes[1]);

  auto &node1 = file.nodes[1];
  CHECK_VEC_CLOSE_ENOUGH(node1.localPosition, Vec3(0, 0, 0), 0.001f);
  CHECK_QUAT_CLOSE_ENOUGH(node1.localOrientation, Quat(0, 0, 0, 1), 0.001f);
  CHECK_VEC_CLOSE_ENOUGH(node1.localScale, Vec3(1, 1, 1), 0.001f);
  CHECK(node1.idxSelf == 1);
  CHECK(node1.mesh == &file.meshes[0]);
  CHECK(node1.camera == nullptr);
  CHECK(node1.light == nullptr);
  CHECK(node1.skin == nullptr);
  CHECK(node1.parent == &node0);
  CHECK(node1.children.empty());
}

SN_TEST(Gltf, Box_scenes) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_GLB);

  CHECK(file.scenes.length == 1);

  CHECK(file.scenes[0].rootNodes.length == 1);
  CHECK(file.scenes[0].rootNodes[0] == &file.nodes[0]);

  CHECK(file.scene == &file.scenes[0]);
}

SN_TEST(Gltf, BoxTextured_texcoord) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_TEXTURED_GLB);

  CHECK(file.meshes.length == 1);

  auto &mesh0 = file.meshes[0];
  CHECK(mesh0.primitives[0].texcoord0 != nullptr);
  auto *texcoord = mesh0.primitives[0].texcoord0;
  CHECK(texcoord->bufferView == &file.bufferViews[2]);
  CHECK(texcoord->byteOffset == 0);
  CHECK(texcoord->componentType == sn::gltf::GL_FLOAT);
  CHECK(texcoord->count == 24);
  CHECK(texcoord->type == sn::gltf::AccessorType::VEC2);
}

SN_TEST(Gltf, BoxTextured_textures) {
  Arena::Scope temp = getScratch(nullptr, 0);
  sn::gltf::File file = parseModel(temp, BOX_TEXTURED_GLB);

  CHECK(file.textures.length == 1);
  CHECK(file.samplers.length == 1);
  CHECK(file.images.length == 1);
  CHECK(file.materials.length == 1);

  CHECK(file.textures[0].source == &file.images[0]);
  CHECK(file.textures[0].sampler == &file.samplers[0]);

  CHECK(file.materials[0].metallicRoughness.hasValue());
  CHECK(file.materials[0].metallicRoughness->baseColorMap.hasValue());
  CHECK(file.materials[0].metallicRoughness->baseColorMap->texture ==
        &file.textures[0]);
  CHECK(file.materials[0].metallicRoughness->baseColorMap->texcoord == 0);

  CHECK(file.samplers[0].magFilter == sn::gltf::GL_LINEAR);
  CHECK(file.samplers[0].minFilter == sn::gltf::GL_NEAREST_MIPMAP_LINEAR);
  CHECK(file.samplers[0].wrapS == sn::gltf::GL_REPEAT);
  CHECK(file.samplers[0].wrapT == sn::gltf::GL_REPEAT);
}

static inline f32 RadicalInverse_VdC(u32 bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return ((f32)bits) * 2.3283064365386963e-10f;  // / 0x100000000
}

static inline void test_quat_matrix_conversion_roundtrip(const Quat &expected) {
  f32 M[9];
  sn::gltf::rotationMatrixFrom(M, expected);

  Quat actual = sn::gltf::quatFromRotationMatrix(M);

#if 0
  printf("[%f %f %f %f] [%f %f %f %f]\n", q.w, q.x, q.y, q.z, actual.w,
         actual.x, actual.y, actual.z);
#endif

  CHECK_QUAT_CLOSE_ENOUGH(actual, expected, 0.001f);
}

SN_TEST(Gltf, quatFromMatrix) {
  // Test matrix -> quaternion conversion

  test_quat_matrix_conversion_roundtrip(Quat(0, 0, 0, 1));
  test_quat_matrix_conversion_roundtrip(Quat(1, 0, 0, 0));
  test_quat_matrix_conversion_roundtrip(Quat(0, 1, 0, 0));
  test_quat_matrix_conversion_roundtrip(Quat(0, 0, 1, 0));

  test_quat_matrix_conversion_roundtrip(Quat(0, 0, 0, -1));
  test_quat_matrix_conversion_roundtrip(Quat(-1, 0, 0, 0));
  test_quat_matrix_conversion_roundtrip(Quat(0, -1, 0, 0));
  test_quat_matrix_conversion_roundtrip(Quat(0, 0, -1, 0));

  test_quat_matrix_conversion_roundtrip(Quat(0.707f, 0, 0, 0.707f));
  test_quat_matrix_conversion_roundtrip(Quat(0, 0.707f, 0, 0.707f));
  test_quat_matrix_conversion_roundtrip(Quat(0, 0, 0.707f, 0.707f));

  test_quat_matrix_conversion_roundtrip(Quat(0.707f, 0, 0, -0.707f));
  test_quat_matrix_conversion_roundtrip(Quat(0, 0.707f, 0, -0.707f));
  test_quat_matrix_conversion_roundtrip(Quat(0, 0, 0.707f, -0.707f));

  // Random quaternions
  for (u32 i = 0; i < 4096; i++) {
    f32 w = 2.0f * RadicalInverse_VdC(i + 13) - 1.0f;
    f32 x = 2.0f * RadicalInverse_VdC(i + 7) - 1.0f;
    f32 y = 2.0f * RadicalInverse_VdC(i + 41) - 1.0f;
    f32 z = 2.0f * RadicalInverse_VdC(i + 19) - 1.0f;
    f32 len = sqrtf(w * w + x * x + y * y + z * z);
    w /= len;
    x /= len;
    y /= len;
    z /= len;
    Quat q = Quat(x, y, z, w);
    test_quat_matrix_conversion_roundtrip(q);
  }
}
