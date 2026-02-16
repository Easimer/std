#pragma once

#include "std/Arena.h"
#include "std/Slice.hpp"
#include "std/json/Value.hpp"

bool tryParseValue(Arena *arena,
                   Arena *tempArena,
                   Slice<const char> &json,
                   JsonValue &out);
bool tryParseValue(Arena *arena, Slice<const char> &json, JsonValue &out);
