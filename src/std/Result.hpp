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

  ResultStorage() = delete;

  ResultStorage(const T &value) : value(value), isValue(true) {}
  ResultStorage(const E &error) : error(error), isValue(false) {}

  ResultStorage(const ResultStorage<T, E> &other) : isValue(other.isValue) {
    if (other.isValue) {
      new (&value) T(other.value);
    } else {
      new (&error) E(other.error);
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

  void operator=(const T &) = delete;
  void operator=(T &&) = delete;

  void operator=(const E &) = delete;
  void operator=(E &&) = delete;
};
}  // namespace impl

template <typename T, typename E>
struct Result {
  impl::ResultStorage<T, E> storage;

  ~Result() = default;

  Result() = delete;
  constexpr Result(const T &other) : storage(other) {}
  constexpr Result(const E &other) : storage(other) {}
  constexpr Result(const Result<T, E> &other) : storage(other.storage) {}
  constexpr Result(T &&other) noexcept : storage(std::move(other)) {}
  constexpr Result(E &&other) noexcept : storage(std::move(other)) {}
  constexpr Result(Result<T, E> &&other) noexcept
      : storage(std::move(other.storage)) {}

  constexpr Result<T, E> &operator=(const Result<T, E> &other) {
    storage = other.storage;
    return *this;
  }

  constexpr Result<T, E> &operator=(Result<T, E> &&other) noexcept {
    storage = std::move(other.storage);
    return *this;
  }

  void operator=(const T &) = delete;
  void operator=(T &&) = delete;

  void operator=(const E &) = delete;
  void operator=(E &&) = delete;

  constexpr bool isOk() const noexcept { return storage.isValue; }
  constexpr bool isErr() const noexcept { return !storage.isValue; }
  constexpr explicit operator bool() const noexcept { return isOk(); }

  T *operator->() noexcept {
    DCHECK(isOk());
    return &storage.value;
  }

  T &unwrap() noexcept {
    DCHECK(isOk());
    return storage.value;
  }

  T unwrapOrDefault() noexcept {
    if (isOk()) {
      return unwrap();
    }

    return T();
  }

  E &unwrapErr() noexcept {
    DCHECK(isErr());
    return storage.error;
  }
};

template <typename T, typename E>
Result<T, E> flatten(Result<Result<T, E>, E> res) {
  if (res.isErr()) {
    return res.unwrapErr();
  }

  Result<T, E> &inner = res.unwrap();
  if (inner.isErr()) {
    return inner.unwrapErr();
  }

  return inner.unwrap();
}
