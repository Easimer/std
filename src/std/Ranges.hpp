/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Types.h"

#include <concepts>

template <typename C>
concept HasLength =
    requires(C c) { requires std::is_integral_v<decltype(c.length)>; };

template <typename C>
concept ContainerWithLength = requires(C c, u32 i) {
  typename C::KeyT;
  typename C::ValueT;
  requires HasLength<C>;

  { c.data } -> std::same_as<typename C::ValueT *&>;
};

template <ContainerWithLength C>
struct ArrayKeyIterator {
  using KeyT = typename C::KeyT;
  using ValueT = typename C::ValueT;

  using value_type = KeyT;
  using difference_type = ptrdiff_t;
  using reference = KeyT &;
  using pointer = KeyT *;
  using iterator_category = std::forward_iterator_tag;

  ValueT *const data = nullptr;
  KeyT index = 0;

  bool operator==(const ArrayKeyIterator<C> &other) const {
    return index == other.index;
  }

  bool operator!=(const ArrayKeyIterator<C> &other) const {
    return !((*this) == other);
  }

  void operator++() { index++; }

  KeyT operator*() const { return index; }
};

template <ContainerWithLength C>
struct ArrayValueIterator {
  using KeyT = typename C::KeyT;
  using ValueT = typename C::ValueT;

  using value_type = ValueT;
  using difference_type = ptrdiff_t;
  using reference = ValueT &;
  using pointer = ValueT *;
  using iterator_category = std::forward_iterator_tag;

  ValueT *const data = nullptr;
  KeyT index = 0;

  bool operator==(const ArrayValueIterator<C> &other) const {
    return index == other.index;
  }

  bool operator!=(const ArrayValueIterator<C> &other) const {
    return !((*this) == other);
  }

  void operator++() { index++; }

  ValueT &operator*() const { return data[index]; }
};

template <ContainerWithLength C>
struct ArrayEntryIterator {
  using KeyT = typename C::KeyT;
  using ValueT = typename C::ValueT;

  struct Element {
    KeyT index;
    ValueT &value;
  };

  using value_type = Element;
  using difference_type = ptrdiff_t;
  using reference = Element &;
  using pointer = Element *;
  using iterator_category = std::forward_iterator_tag;

  ValueT *const data = nullptr;
  KeyT index = 0;

  bool operator==(const ArrayEntryIterator<C> &other) const {
    return index == other.index;
  }

  bool operator!=(const ArrayEntryIterator<C> &other) const {
    return !((*this) == other);
  }

  void operator++() { index++; }

  Element operator*() const { return {index, data[index]}; }
};

template <typename It>
struct ArrayIteratorMaker {
  using KeyT = typename It::KeyT;
  using ValueT = typename It::ValueT;
  ValueT *const data = nullptr;
  KeyT const length = 0;

  It begin() { return {data, 0}; }
  It end() { return {data, length}; }
  using iterator = It;
};

template <ContainerWithLength C>
ArrayIteratorMaker<ArrayKeyIterator<C>> keysOf(const C &array) {
  return {array.data, array.length};
}

template <ContainerWithLength C>
ArrayIteratorMaker<ArrayValueIterator<C>> valuesOf(const C &array) {
  return {array.data, array.length};
}

template <ContainerWithLength C>
ArrayIteratorMaker<ArrayEntryIterator<C>> entriesOf(const C &array) {
  return {array.data, array.length};
}

#define SN_RANGES_ITERATOR_TYPEDEFS(BaseIterator)       \
  using value_type = typename BaseIterator::value_type; \
  using difference_type = ptrdiff_t;                    \
  using reference = typename BaseIterator::reference;   \
  using pointer = typename BaseIterator::pointer;       \
  using iterator_category = std::input_iterator_tag;

#define SN_RANGES_ITERATOR_EQ_FROM_NEQ(Self) \
  bool operator==(const Self &other) const { \
    return !((*this) != other);              \
  }

template <typename R, typename F>
struct TakeWhileView {
  using BaseIterator = typename std::remove_reference_t<R>::iterator;

  R _base;
  F _cond;

  TakeWhileView(R &&base, F &&cond)
      : _base(std::forward<R>(base)), _cond(std::move(cond)) {}

  struct Iterator {
    SN_RANGES_ITERATOR_TYPEDEFS(BaseIterator);
    SN_RANGES_ITERATOR_EQ_FROM_NEQ(Iterator);

    BaseIterator _base;
    BaseIterator _end;
    F *_cond;

    explicit Iterator() : _cond(nullptr) {}
    Iterator(BaseIterator base, BaseIterator end, F &cond)
        : _base(base), _end(end), _cond(&cond) {}
    bool sentinel() const { return _cond == nullptr; }

    Iterator &operator++() {
      ++_base;
      return *this;
    }

    auto operator*() const { return *_base; }

    bool operator!=(const Iterator &other) const {
      DCHECK(!sentinel() && other.sentinel());
      return _base != _end && (*_cond)(*_base);
    }
  };
  using iterator = Iterator;

  Iterator begin() { return Iterator(_base.begin(), _base.end(), _cond); }
  Iterator end() { return Iterator(); }
};

template <typename F>
struct takeWhile {
  F _cond;
  takeWhile(F &cond) : _cond(cond) {}
  takeWhile(F &&cond) : _cond(cond) {}

  template <typename R>
  TakeWhileView<R, F> operator()(R &&r) {
    return TakeWhileView<R, F>(std::forward<R>(r), std::move(_cond));
  }
};

template <typename R, typename F>
struct FilterView {
  using BaseIterator = typename std::remove_reference_t<R>::iterator;

  R _base;
  F _cond;

  FilterView(R &&base, F &&cond)
      : _base(std::forward<R>(base)), _cond(std::move(cond)) {}

  struct Iterator {
    SN_RANGES_ITERATOR_TYPEDEFS(BaseIterator);
    SN_RANGES_ITERATOR_EQ_FROM_NEQ(Iterator);

    BaseIterator _base;
    BaseIterator _end;
    F *_cond;

    explicit Iterator() : _cond(nullptr) {}
    Iterator(BaseIterator base, BaseIterator end, F &cond)
        : _base(base), _end(end), _cond(&cond) {}
    bool sentinel() const { return _cond == nullptr; }

    Iterator &operator++() {
      do {
        ++_base;
      } while (_base != _end && !(*_cond)(*_base));
      return *this;
    }

    auto operator*() const { return *_base; }
    bool operator!=(const Iterator &other) const {
      DCHECK(!sentinel() && other.sentinel());
      return _base != _end;
    }
  };

  Iterator begin() {
    auto base = _base.begin();
    auto end = _base.end();
    // Step the iterator until the condition becomes true, or reaches the end
    while (base != end && !_cond(*base)) {
      ++base;
    }
    return Iterator(base, end, _cond);
  }
  Iterator end() { return Iterator(); }
};

template <typename F>
struct filter {
  F _cond;
  filter(F &cond) : _cond(cond) {}
  filter(F &&cond) : _cond(cond) {}

  template <typename R>
  FilterView<R, F> operator()(R &&r) {
    return FilterView<R, F>(std::forward<R>(r), std::move(_cond));
  }
};

template <class R, typename F>
struct MapView {
  using BaseIterator = typename std::remove_reference_t<R>::iterator;

  R _base;
  F _transform;

  MapView(R &&base, F &&transform)
      : _base(std::forward<R>(base)), _transform(std::move(transform)) {}

  struct Iterator {
    SN_RANGES_ITERATOR_TYPEDEFS(BaseIterator);
    SN_RANGES_ITERATOR_EQ_FROM_NEQ(Iterator);

    BaseIterator _base;
    BaseIterator _end;
    F *_transform;

    explicit Iterator() : _transform(nullptr) {}
    Iterator(BaseIterator base, BaseIterator end, F &transform)
        : _base(base), _end(end), _transform(&transform) {}

    bool sentinel() const { return _transform == nullptr; }

    Iterator &operator++() {
      ++_base;
      return *this;
    }

    auto operator*() const { return (*_transform)(*_base); }
    bool operator!=(const Iterator &other) const {
      DCHECK(!sentinel() && other.sentinel());
      return _base != _end;
    }
  };

  using iterator = Iterator;

  Iterator begin() { return Iterator(_base.begin(), _base.end(), _transform); }
  Iterator end() { return Iterator(); }
};

template <typename F>
struct map {
  F _transform;
  map(F &transform) : _transform(transform) {}
  map(F &&transform) : _transform(transform) {}

  template <typename R>
  MapView<R, F> operator()(R &&r) {
    return MapView<R, F>(std::forward<R>(r), std::move(_transform));
  }
};

template <typename R>
struct TakeView {
  using BaseIterator = typename std::remove_reference_t<R>::iterator;

  R _base;
  size_t _num;

  TakeView(R &&base, size_t num) : _base(std::forward<R>(base)), _num(num) {}

  struct Iterator {
    SN_RANGES_ITERATOR_TYPEDEFS(BaseIterator);
    SN_RANGES_ITERATOR_EQ_FROM_NEQ(Iterator);

    BaseIterator _base;
    BaseIterator _end;
    size_t _remains;

    Iterator() : _remains(0) {}
    Iterator(BaseIterator base, BaseIterator end, size_t remains)
        : _base(base), _end(end), _remains(remains) {}
    bool sentinel() const noexcept { return _remains == 0; }

    Iterator &operator++() {
      ++_base;
      --_remains;
      return *this;
    }

    auto operator*() const { return *_base; }
    bool operator!=(const Iterator &other) const {
      DCHECK(other.sentinel());
      return _base != _end && _remains > 0;
    }
  };

  Iterator begin() { return Iterator(_base.begin(), _base.end(), _num); }
  Iterator end() { return Iterator(); }

  using iterator = Iterator;
};

struct take {
  size_t _num;
  take(size_t num) : _num(num) {}

  template <typename R>
  TakeView<R> operator()(R &&r) {
    return TakeView<R>(std::forward<R>(r), _num);
  }
};

struct count {
  template <typename R>
  size_t operator()(R &&r) {
    auto it = r.begin();
    auto end = r.end();
    size_t ret = 0;
    while (it != end) {
      ++ret;
      ++it;
    }
    return ret;
  }
};

template <typename R, typename Adaptor>
constexpr auto operator|(R &&r, Adaptor &&adaptor) {
  return adaptor(std::forward<R>(r));
}

template <typename R>
auto tryGetFirst(R &&r) {
  auto it = r.begin();
  auto end = r.end();
  if (it != end) {
    return Optional(*it);
  }

  using E = decltype(*it);
  return Optional<E>();
}

template <typename R, typename T>
T getFirstOr(R &&r, T def) {
  auto it = r.begin();
  auto end = r.end();
  if (it != end) {
    return *it;
  }

  return def;
}
