#pragma once

#include "./Types.h"

namespace sn {
struct Allocator {
  virtual u8 *alloc(size_t byteLength, size_t alignment) noexcept = 0;
  virtual bool resize(u8 *allocation,
                      size_t alignment,
                      size_t nextLength) noexcept = 0;
  virtual u8 *remap(u8 *allocation,
                    size_t alignment,
                    size_t nextLength) noexcept = 0;
  virtual void free(u8 *allocation, size_t alignment) noexcept = 0;

  template <typename T, size_t Align>
  T *alloc(size_t count = 1) noexcept {
    void *ptr = alloc(count * sizeof(T), Align);
    return reinterpret_cast<T *>(SN_ASSUME_ALIGNED(ptr, Align));
  }

  template <typename T>
  T *alloc(size_t count = 1) noexcept {
    return alloc<T, alignof(T)>(count);
  }

  template <typename T, size_t Align>
  void free(T *allocation) noexcept {
    free(reinterpret_cast<u8*>(allocation), Align);
  }

  template <typename T>
  void free(T *allocation) noexcept {
    return free<T, alignof(T)>(allocation);
  }
};

}  // namespace sn
