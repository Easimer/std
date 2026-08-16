#pragma once

#include "std/CompilerInfo.h"

#if defined(__SSE4_2__) || defined(_M_X64)
#include "f32x4_sse42.hpp"
using f32x4 = sse42::f32x4;
#else
#include "f32x4_scalar.hpp"
using f32x4 = scalar::f32x4;
#endif

