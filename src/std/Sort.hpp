/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"

namespace impl {

template <typename T>
struct Less {
  constexpr bool operator()(const T &lhs, const T &rhs) const {
    return lhs < rhs;
  }
};

template <typename T, typename Cmp>
void merge(MutSlice<T> dst, Slice<T> left, Slice<T> right, const Cmp &cmp) {
  size_t idxLeft = 0;
  size_t idxRight = 0;

  for (size_t idxDst = 0; idxDst < dst.length; idxDst++) {
    if (idxLeft < left.length &&
        (idxRight == right.length || cmp(left[idxLeft], right[idxRight]))) {
      dst[idxDst] = left[idxLeft++];
    } else {
      dst[idxDst] = right[idxRight++];
    }
  }
}

template <typename T, typename Cmp>
void mergeSort(MutSlice<T> dst, MutSlice<T> s, const Cmp &cmp) {
  if (s.length == 1) {
    return;
  }

  MutSlice<T> left, right;
  MutSlice<T> dstLeft, dstRight;

  left.data = s.data;
  left.length = s.length / 2;

  right.data = s.data + left.length;
  right.length = s.length - left.length;

  dstLeft.data = dst.data;
  dstLeft.length = left.length;

  dstRight.data = dst.data + left.length;
  dstRight.length = right.length;

  ::impl::mergeSort<T, Cmp>(left, dstLeft, cmp);
  ::impl::mergeSort<T, Cmp>(right, dstRight, cmp);

  merge(dst, left.asSlice(), right.asSlice(), cmp);
}

}  // namespace impl

template <typename T, typename Cmp = impl::Less<T>>
void mergeSort(MutSlice<T> dst, MutSlice<T> s, const Cmp &cmp) {
  DCHECK(dst.length == s.length);
  if (s.empty()) {
    return;
  }
  dst.copy(s);
  impl::mergeSort<T>(dst, s, cmp);
}

template <typename T>
void mergeSort(MutSlice<T> s) {
  if (s.empty()) {
    return;
  }
  Arena::Scope temp = getScratch(nullptr, 0);
  MutSlice<T> copy = duplicate<T>(temp, s);
  ::impl::mergeSort<T>(s, copy, impl::Less<T>{});
}

template <typename T>
void mergeSort(MutSlice<T> dst, Slice<T> s) {
  DCHECK(dst.length == s.length);
  if (s.empty()) {
    return;
  }
  dst.copy(s);
  ::impl::mergeSort<T>(dst, s, impl::Less<T>{});
}

template <typename T>
void mergeSort(MutSlice<T> dst, MutSlice<T> s) {
  DCHECK(dst.length == s.length);
  if (s.empty()) {
    return;
  }
  dst.copy(s);
  ::impl::mergeSort<T>(dst, s, impl::Less<T>{});
}

template <typename T, typename Cmp = impl::Less<T>>
void mergeSort(MutSlice<T> s, const Cmp &cmp) {
  if (s.empty()) {
    return;
  }

  Arena::Scope temp = getScratch(nullptr, 0);
  MutSlice<T> copy = duplicate<T>(temp, s);

  ::impl::mergeSort<T, Cmp>(s, copy, cmp);
}

