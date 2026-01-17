/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "./Check.h"
#include "./Slice.hpp"
#include "./Types.h"

struct Uuid {
  enum Variant : u8 {
    VARIANT_UNKNOWN,
    VARIANT_NCS,
    VARIANT_DCE,
    VARIANT_MS,
  };

  struct Version4 {};
  struct Version7 {};
  struct Version8 {};

  /**
   * \brief Octets stored in little-endian order
   */
  u8 value[16];

  constexpr Uuid() : Uuid(0, 0, 0, 0, 0) {}

  /**
   * \brief Construct a UUID from its 5 parts:
   * \param w32 time_low
   * \param w1 time_mid
   * \param w2 time_hi_and_version
   * \param w3 clock_seq_hi_and_reserved and clock_seq_low
   * \param w48 node (the top 16 bits are ignored)
   */
  constexpr Uuid(u32 w32, u16 w1, u16 w2, u16 w3, u64 w48)
      : value{
            static_cast<u8>((w48 >> 0) & 0xFFU),   //
            static_cast<u8>((w48 >> 8) & 0xFFU),   //
            static_cast<u8>((w48 >> 16) & 0xFFU),  //
            static_cast<u8>((w48 >> 24) & 0xFFU),  //
            static_cast<u8>((w48 >> 32) & 0xFFU),  //
            static_cast<u8>((w48 >> 40) & 0xFFU),  //
                                                   //
            static_cast<u8>((w3 >> 0) & 0xFFU),    //
            static_cast<u8>((w3 >> 8) & 0xFFU),    //
            //
            static_cast<u8>((w2 >> 0) & 0xFFU),  //
            static_cast<u8>((w2 >> 8) & 0xFFU),  //
            //
            static_cast<u8>((w1 >> 0) & 0xFFU),  //
            static_cast<u8>((w1 >> 8) & 0xFFU),  //
            //
            static_cast<u8>((w32 >> 0) & 0xFFU),   //
            static_cast<u8>((w32 >> 8) & 0xFFU),   //
            static_cast<u8>((w32 >> 16) & 0xFFU),  //
            static_cast<u8>((w32 >> 24) & 0xFFU),  //
        } {}

  constexpr Uuid(Version4, u64 randomA, u16 randomB, u64 randomC)
      : Uuid(
            /* w32: */ (randomA >> 16) & 0xFFFFFFFFU,
            /* w1: */ (randomA >> 0) & 0xFFFFU,
            /* w2: */ (0x4000) | ((randomB >> 0) & 0x0FFF),
            /* w3: */ (0x8000) | ((randomC >> 48) & 0x3FFF),
            /* w48: */ (randomC >> 0) & 0xFFFFFFFFFFFFULL) {}

  constexpr Uuid(Version7, u64 unixTimestampMs, u16 randomA, u64 randomB)
      : Uuid(
            /* w32: */ (unixTimestampMs >> 16) & 0xFFFFFFFFU,
            /* w1: */ (unixTimestampMs >> 0) & 0xFFFFU,
            /* w2: */ (0x7000) | ((randomA >> 0) & 0x0FFF),
            /* w3: */ (0x8000) | ((randomB >> 48) & 0x3FFF),
            /* w48: */ (randomB >> 0) & 0xFFFFFFFFFFFFULL) {}

  constexpr Uuid(Version8, u64 customA, u16 customB, u64 customC)
      : Uuid(
            /* w32: */ (customA >> 16) & 0xFFFFFFFFU,
            /* w1: */ (customA >> 0) & 0xFFFFU,
            /* w2: */ (0x8000) | ((customB >> 0) & 0x0FFF),
            /* w3: */ (0x8000) | ((customC >> 48) & 0x3FFF),
            /* w48: */ (customC >> 0) & 0xFFFFFFFFFFFFULL) {}

  operator Slice<u8>() noexcept { return sliceFrom(value); }
  operator Slice<const u8>() const noexcept { return sliceFrom(value); }

  bool operator==(const Uuid &rhs) const noexcept {
    return sliceFrom(value) == sliceFrom(rhs.value);
  }

  /**
   * \brief Accesses the raw bytes of the UUID in the same order as they would
   * be shown in the hex-and-dash format.
   */
  u8 operator[](u32 i) const noexcept {
    DCHECK(i < 16);
    return value[15 - i];
  }

  constexpr u8 rawVariant() const noexcept { return (value[7] >> 4) & 0xFU; }
  constexpr Variant variant() const noexcept {
    u8 rv = rawVariant();
    if ((rv & 0b1000) == 0b0000) {
      return VARIANT_NCS;
    }

    if ((rv & 0b1100) == 0b1000) {
      return VARIANT_DCE;
    }

    if ((rv & 0b1110) == 0b1100) {
      return VARIANT_MS;
    }

    return VARIANT_UNKNOWN;
  }

  constexpr u8 version() const noexcept {
    DCHECK(variant() == VARIANT_DCE);
    return (value[9] >> 4) & 0xFU;
  }
};
