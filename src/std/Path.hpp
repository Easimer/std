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

Slice<const char> dirname(Slice<const char> path);
Slice<const char> basename(Slice<const char> path);
Slice<char> joinSimple(Arena *arena,
                       Slice<const char> segment0,
                       Slice<const char> segment1);
