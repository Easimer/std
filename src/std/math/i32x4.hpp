#pragma once

#include "std/CompilerInfo.h"

#if defined(__SSE4_2__) || defined(_M_X64)
#include "i32x4_sse42.hpp"
using i32x4 = sse42::i32x4;
#else
#include "i32x4_scalar.hpp"
using i32x4 = scalar::i32x4;
#endif
