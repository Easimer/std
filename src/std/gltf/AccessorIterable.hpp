#pragma once

#include "gltf2/Descriptor.hpp"
#include "std/Slice.hpp"
#include "std/Types.h"

namespace sn::gltf {

template <typename T>
struct AccessorIterable {
  struct Iterator {
    Slice<const u8> buffer;
    const u32 sizStride;

    const T &operator*() const {
      Slice<const u8> bufAttr = buffer.subarray(0, sizeof(T));
      return bufAttr.cast<const T>()[0];
    }

    void operator++() { buffer.shrinkFromLeftByCount(sizStride); }

    bool operator!=(const Iterator &other) const {
      // NOTE(danielm): assuming here that both this and other are iterating
      // over the same accessor
      return buffer.length != other.buffer.length;
    }
  };

  const Iterator _begin;
  const Iterator _end;

  AccessorIterable(const sn::gltf::Accessor *acc)
      : AccessorIterable(
            // Get a slice for the buffer view then advance the slice by the
            // accessor offset, so that the slice points at the first byte of
            // the first vertex attribute
            acc->bufferView->slice().subarray(
                acc->byteOffset,
                acc->byteOffset +
                    AccessorIterable<T>::strideOf(acc) * acc->count),
            AccessorIterable<T>::strideOf(acc)) {}

  AccessorIterable(Slice<const u8> buffer, u32 sizStride)
      : _begin({buffer, sizStride}), _end({{nullptr, 0}, sizStride}) {}

  Iterator begin() { return _begin; }
  Iterator end() { return _end; }

  static u32 strideOf(const sn::gltf::Accessor *accessor) {
    const sn::gltf::BufferView *bufferView = accessor->bufferView;
    if (bufferView->byteStride == 0) {
      return sizeof(T);
    }

    return bufferView->byteStride;
  }
};

namespace impl {
/**
 * Iterates over the data described by an accessor, converts the input (vectors
 * of `ComponentIn`) to the output format (vec4 of `ComponentOut`) and passes it
 * to the provided Callable.
 * \param dst Output array; iterated along with the accessor and it's current
 * item is passed by reference to the Callable
 * \param accessor Accessor
 * \param C A callable with two parameters:
 * `(Out& curOut, glm::vec<4, ComponentOut> v) -> void`
 */
template <typename ComponentOut,
          typename ComponentIn,
          typename Out,
          typename Callable>
static bool forEachVertex(Slice<Out> dst, Accessor *accessor, Callable &&C) {
  Slice<Out> rest = dst;
  switch (accessor->type) {
    case gltf::AccessorType::SCALAR: {
      for (auto &v : AccessorIterable<glm::vec<1, ComponentIn>>(accessor)) {
        C(rest[0], glm::vec<4, ComponentOut>(v.x, 0, 0, 0));
        rest.shrinkFromLeft();
      }
      break;
    }
    case gltf::AccessorType::VEC2: {
      for (auto &v : AccessorIterable<glm::vec<2, ComponentIn>>(accessor)) {
        C(rest[0], glm::vec<4, ComponentOut>(v.x, v.y, 0, 0));
        rest.shrinkFromLeft();
      }
      break;
    }
    case gltf::AccessorType::VEC3: {
      for (auto &v : AccessorIterable<glm::vec<3, ComponentIn>>(accessor)) {
        C(rest[0], glm::vec<4, ComponentOut>(v.x, v.y, v.z, 0));
        rest.shrinkFromLeft();
      }
      break;
    }
    case gltf::AccessorType::VEC4: {
      for (auto &v : AccessorIterable<glm::vec<4, ComponentIn>>(accessor)) {
        C(rest[0], glm::vec<4, ComponentOut>(v.x, v.y, v.z, v.w));
        rest.shrinkFromLeft();
      }
      break;
    }
    default: {
      return false;
    }
  }

  DCHECK(rest.empty());
  return true;
}
}  // namespace impl

template <typename Out, typename Callable>
static void forEachVertex(Slice<Out> dst, Accessor *accessor, Callable &&C) {
  u16 componentType = accessor->componentType;

  switch (componentType) {
    case GL_FLOAT: {
      impl::forEachVertex<f32, f32>(dst, accessor, C);
      break;
    }
    default: {
      TODO();
      break;
    }
  }
}

template <typename Out, typename Callable>
static void forEachVertexUnsigned(Slice<Out> dst,
                                  Accessor *accessor,
                                  Callable &&C) {
  u16 componentType = accessor->componentType;

  switch (componentType) {
    case GL_UNSIGNED_BYTE: {
      impl::forEachVertex<u32, u8>(dst, accessor, C);
      break;
    }
    case GL_UNSIGNED_SHORT: {
      impl::forEachVertex<u32, u16>(dst, accessor, C);
      break;
    }
    case GL_UNSIGNED_INT: {
      impl::forEachVertex<u32, u32>(dst, accessor, C);
      break;
    }
    default: {
      TODO();
      break;
    }
  }
}

}  // namespace sn::gltf
