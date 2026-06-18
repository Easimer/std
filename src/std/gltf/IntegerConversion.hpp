#pragma once

#include "std/Types.h"

#include <math.h>

template <typename T, T Scale>
T unormFrom(f32 v) {
  if (isnan(v)) {
    return 0;
  }

  if (v > 1.0f) {
    v = 1.0f;
  }
  if (v < 0.0f) {
    v = 0.0f;
  }

  v = v * f32(Scale);
  return (T)(v + 0.5f);
}

template <typename T, T Scale>
T snormFrom(f32 v) {
  if (isnan(v)) {
    return 0;
  }

  if (v > 1.0f) {
    v = 1.0f;
  }
  if (v < -1.0f) {
    v = -1.0f;
  }

  v = v * f32(Scale);

  if (v >= 0) {
    return (T)(v + 0.5f);
  } else {
    return (T)(v - 0.5f);
  }
}

template <typename T, T Scale>
f32 floatFromUnorm(T v) {
  f32 t = (f32)v;
  return t / f32(Scale);
}

template <typename T, T Scale>
f32 floatFromSnorm(T v) {
  // The most-negative value maps to -1.0f
  if (v == (-Scale - 1)) {
    return -1.0f;
  }

  f32 t = (f32)v;
  return t / f32(Scale);
}

template <bool Normalized, bool Signed, typename T, T Scale>
struct IntegerConversionImpl {};

/**
 * \brief IntegerConversion impl for unnormalized types
 */
template <bool Signed, typename T, T Scale>
struct IntegerConversionImpl<false, Signed, T, Scale> {
  f32 floatFrom(T v) const { return static_cast<f32>(v); }
  T intFrom(f32 v) const { return static_cast<T>(v); }
};

/**
 * \brief IntegerConversion impl for unsigned normalized types
 */
template <typename T, T Scale>
struct IntegerConversionImpl<true, false, T, Scale> {
  f32 floatFrom(T v) const { return floatFromSnorm<T, Scale>(v); }
  T intFrom(f32 v) const { return unormFrom<T, Scale>(v); }
};

/**
 * \brief IntegerConversion impl for signed normalized types
 */
template <typename T, T Scale>
struct IntegerConversionImpl<true, true, T, Scale> {
  f32 floatFrom(T v) const { return floatFromSnorm<T, Scale>(v); }
  T intFrom(f32 v) const { return snormFrom<T, Scale>(v); }
};

template <typename T, bool Normalized>
struct IntegerConversion {};

template <bool Normalized>
struct IntegerConversion<u8, Normalized>
    : IntegerConversionImpl<Normalized, false, u8, 0xFF> {};

template <bool Normalized>
struct IntegerConversion<i8, Normalized>
    : IntegerConversionImpl<Normalized, true, i8, 0x7F> {};

template <bool Normalized>
struct IntegerConversion<u16, Normalized>
    : IntegerConversionImpl<Normalized, false, u16, 0xFFFF> {};

template <bool Normalized>
struct IntegerConversion<i16, Normalized>
    : IntegerConversionImpl<Normalized, true, i16, 0x7FFF> {};

template <bool Normalized>
struct IntegerConversion<u32, Normalized>
    : IntegerConversionImpl<Normalized, false, u32, 0xFFFFFFFF> {};

template <bool Normalized>
struct IntegerConversion<i32, Normalized>
    : IntegerConversionImpl<Normalized, true, i32, 0x7FFFFFFF> {};
