/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace impl {
template <typename Func>
struct Defer {
  Func func;
  Defer(Func f) : func(f) {}
  ~Defer() { func(); }
};

template <typename Func>
Defer<Func> makeDefer(Func f) {
  return Defer<Func>(f);
}
}  // namespace impl

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define DEFER(expr) auto DEFER_3(_defer) = impl::makeDefer([&]() { expr; })
