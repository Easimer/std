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
void merge(Slice<T> dst, Slice<T> left, Slice<T> right, Cmp &&cmp) {
  u32 idxLeft = 0;
  u32 idxRight = 0;

  for (u32 idxDst = 0; idxDst < dst.length; idxDst++) {
    if (idxLeft < left.length &&
        (idxRight == right.length || cmp(left[idxLeft], right[idxRight]))) {
      dst[idxDst] = left[idxLeft++];
    } else {
      dst[idxDst] = right[idxRight++];
    }
  }
}

template <typename T, typename Cmp>
void mergeSort(Slice<T> dst, Slice<T> s, Cmp &&cmp) {
  if (s.length == 1) {
    return;
  }

  Slice<T> left, right;
  Slice<T> dstLeft, dstRight;

  left.data = s.data;
  left.length = s.length / 2;

  right.data = s.data + left.length;
  right.length = s.length - left.length;

  dstLeft.data = dst.data;
  dstLeft.length = left.length;

  dstRight.data = dst.data + left.length;
  dstRight.length = right.length;

  ::impl::mergeSort(left, dstLeft, cmp);
  ::impl::mergeSort(right, dstRight, cmp);

  merge(dst, left, right, cmp);
}

}  // namespace impl

template <typename T, typename Cmp = impl::Less<T>>
void mergeSort(Slice<T> dst, Slice<T> s, const Cmp &cmp) {
  DCHECK(dst.length == s.length);
  if (s.empty()) {
    return;
  }
  copy(dst, s);
  impl::mergeSort(dst, s, cmp);
}

template <typename T, typename Cmp = impl::Less<T>>
void mergeSort(Slice<T> s, const Cmp &cmp) {
  if (s.empty()) {
    return;
  }

  Arena::Scope temp = getScratch(nullptr, 0);
  Slice<T> copy = duplicate(temp, s);

  mergeSort(s, copy, cmp);
}

template <typename T>
void mergeSort(Slice<T> dst, Slice<T> s) {
  DCHECK(dst.length == s.length);
  if (s.empty()) {
    return;
  }
  copy(dst, s);
  mergeSort(dst, s, impl::Less<T>{});
}

template <typename T>
void mergeSort(Slice<T> s) {
  if (s.empty()) {
    return;
  }
  Arena::Scope temp = getScratch(nullptr, 0);
  Slice<T> copy = duplicate(temp, s);
  mergeSort(s, copy);
}
