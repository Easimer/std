/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <type_traits>
#include <utility>

#include "std/Check.h"
#include "std/Slice.hpp"

namespace impl {
using namespace std;

template <typename T, typename E = void>
struct OptionalStorage {
  union {
    char dummy;
    T value;
  };

  bool present;

  ~OptionalStorage() {
    if (!present) {
      return;
    }

    value.~T();
  }

  OptionalStorage() : dummy(0), present(false) {}
  OptionalStorage(const T &other) : value(other), present(true) {}
  OptionalStorage(const OptionalStorage<T> &other)
      : value(), present(other.present) {
    if (other.present) {
      value = other.value;
    }
  }
  OptionalStorage(T &&other) : value(std::move(other)), present(true) {}

  constexpr OptionalStorage<T> &operator=(const OptionalStorage<T> &other) {
    if (present) {
      value.~T();
      present = false;
    }

    new (&value) T(other.value);
    present = true;
    return *this;
  }

  constexpr OptionalStorage<T> &operator=(OptionalStorage<T> &&other) {
    if (present) {
      value.~T();
      present = false;
    }

    if (other.present) {
      new (&value) T(std::move(other.value));
      present = true;
    }
    return *this;
  }
};

// Specialization for trivially destructible T's, so that Optional<T> is also
// trivially destructible
template <typename T>
struct OptionalStorage<T, enable_if_t<is_trivially_destructible_v<T>>> {
  union {
    char dummy;
    T value;
  };

  bool present;

  ~OptionalStorage() = default;

  constexpr OptionalStorage() : dummy(0), present(false) {}
  constexpr OptionalStorage(const T &other) : value(other), present(true) {}
  constexpr OptionalStorage(const OptionalStorage<T> &other)
      : value(other.value), present(other.present) {}
  constexpr OptionalStorage(T &&other)
      : value(std::move(other)), present(true) {}

  constexpr OptionalStorage<T> &operator=(const OptionalStorage<T> &other) {
    if (other.present) {
      value = other.value;
    }

    present = other.present;
    return *this;
  }

  constexpr OptionalStorage<T> &operator=(OptionalStorage<T> &&other) {
    present = false;

    if (other.present) {
      value = std::move(other.value);
      present = true;
    }
    return *this;
  }
};

}  // namespace impl

template <typename T>
struct Optional {
  impl::OptionalStorage<T> storage;

  ~Optional() = default;

  constexpr Optional() : storage() {}
  constexpr Optional(const T &other) : storage(other) {}
  constexpr Optional(const Optional<T> &other) : storage(other.storage) {}
  constexpr Optional(T &&other) : storage(std::move(other)) {}

  constexpr Optional<T> &operator=(const Optional<T> &other) {
    reset();
    if (other.hasValue()) {
      storage = impl::OptionalStorage<T>(other.value());
    }

    return *this;
  }

  constexpr Optional<T> &operator=(Optional<T> &&other) {
    if (other.hasValue()) {
      if (hasValue()) {
        storage.value = std::move(other.value());
        other.reset();
      } else {
        T value = std::move(other.value());
        other.reset();
        storage = impl::OptionalStorage<T>(std::move(value));
      }
    } else {
      reset();
    }

    return *this;
  }

  constexpr bool hasValue() const noexcept { return storage.present; }
  constexpr explicit operator bool() const noexcept { return storage.present; }

  void reset() { storage = impl::OptionalStorage<T>(); }

  constexpr T &value() & noexcept {
    CHECK(storage.present);
    return storage.value;
  }

  constexpr T &&value() && noexcept {
    CHECK(storage.present);
    return std::move(storage.value);
  }

  constexpr const T &value() const & noexcept {
    CHECK(storage.present);
    return storage.value;
  }

  constexpr const T &&value() const && noexcept {
    CHECK(storage.present);
    return std::move(storage.value);
  }

  constexpr T valueOr(T fallback) noexcept {
    if (!storage.present) {
      return fallback;
    }

    return storage.value;
  }

  constexpr T *operator->() noexcept { return &value(); }
  constexpr T &operator*() & noexcept { return value(); }
  constexpr T &&operator*() && noexcept { return std::move(value()); }
  constexpr const T *operator->() const noexcept { return &value(); }
  constexpr const T &operator*() const & noexcept { return value(); }
  constexpr const T &&operator*() const && noexcept {
    return std::move(value());
  }

  constexpr operator Slice<T>() noexcept {
    if (!storage.present) {
      return {nullptr, 0};
    }

    return {&storage.value, 1};
  }
};
