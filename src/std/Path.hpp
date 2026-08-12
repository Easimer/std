/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/Arena.h"
#include "std/Slice.hpp"

Slice<char> dirname(Slice<char> path);
Slice<char> basename(Slice<char> path);
MutSlice<char> joinSimple(Arena *arena,
                          Slice<char> segment0,
                          Slice<char> segment1);
