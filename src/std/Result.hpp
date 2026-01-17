/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <new>
#include <type_traits>
#include <utility>

#include "std/Check.h"
#include "std/Slice.hpp"

namespace impl {
using namespace std;
template <typename T, typename E, typename C = void>
struct ResultStorage {
  union {
    T value;
    E error;
  };

  bool isValue;

  ~ResultStorage() {
    if (isValue) {
      value.~T();
    } else {
      error.~E();
    }
  }

  ResultStorage(const T &value) : value(value), isValue(true) {}
  ResultStorage(const E &error) : error(error), isValue(false) {}

  ResultStorage(const ResultStorage<T, E> &other) : isValue(other.isValue) {
    if (other.isValue) {
      value = other.value;
    } else {
      error = other.error;
    }
  }

  ResultStorage(T &&value) : value(std::move(value)), isValue(true) {}
  ResultStorage(E &&error) : error(std::move(error)), isValue(false) {}

  constexpr ResultStorage<T, E> &operator=(const ResultStorage<T, E> &other) {
    if (isValue) {
      value.~T();
    } else {
      error.~E();
    }

    isValue = other.isValue;

    if (other.isValue) {
      new (&value) T(other.value);
    } else {
      new (&error) E(other.error);
    }

    return *this;
  }

  constexpr ResultStorage<T, E> &operator=(ResultStorage<T, E> &&other) {
    if (isValue) {
      value.~T();
    } else {
      error.~E();
    }

    isValue = other.isValue;

    if (other.isValue) {
      new (&value) T(move(other.value));
    } else {
      new (&error) E(move(other.error));
    }

    return *this;
  }
};
}  // namespace impl

template<typename T, typename E>
struct Result {
  impl::ResultStorage<T, E> storage;

  ~Result() = default;

  constexpr Result() : storage() {}
  constexpr Result(const T &other) : storage(other) {}
  constexpr Result(const E &other) : storage(other) {}
  constexpr Result(const Result<T, E> &other) : storage(other.storage) {}
  constexpr Result(T &&other) : storage(std::move(other)) {}
  constexpr Result(E &&other) : storage(std::move(other)) {}
  constexpr Result(Result<T, E> &&other) : storage(std::move(other.storage)) {}

  constexpr Result<T, E> &operator=(const Result<T, E> &other) {
    storage = other.storage;
    return *this;
  }

  constexpr Result<T, E> &operator=(Result<T, E> &&other) {
    storage = std::move(other.storage);
    return *this;
  }

  constexpr bool hasValue() const noexcept { return storage.isValue; }
  constexpr bool hasError() const noexcept { return !storage.isValue; }
  constexpr explicit operator bool() const noexcept { return hasValue(); }
};

