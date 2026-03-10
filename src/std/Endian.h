#pragma once

#include "./Types.h"

#include <string.h>

#if __cplusplus
extern "C" {
#endif

/**
 * \brief Load a signed 16-bit integer stored in big-endian byte order from an
 * aligned address.
 */
static inline i16 li16be_aligned(const i16 *src);
/**
 * \brief Load an unsigned 16-bit integer stored in big-endian byte order from
 * an aligned address.
 */
static inline u16 lu16be_aligned(const u16 *src);
/**
 * \brief Load a signed 16-bit integer stored in big-endian byte order from a
 * possibly unaligned address.
 */
static inline i16 li16be(const u8 *src);
/**
 * \brief Load an unsigned 16-bit integer stored in big-endian byte order from a
 * possibly unaligned address.
 */
static inline u16 lu16be(const u8 *src);

/**
 * \brief Store a signed 16-bit integer in big-endian byte order to an aligned
 * address.
 */
static inline void si16be_aligned(i16 *dst, i16 val);
/**
 * \brief Store an unsigned 16-bit integer in big-endian byte order to an
 * aligned address.
 */
static inline void su16be_aligned(u16 *dst, u16 val);
/**
 * \brief Store a signed 16-bit integer in big-endian byte order to a
 * (possibly) unaligned address.
 */
static inline void si16be(u8 *dst, i16 val);
/**
 * \brief Store an unsigned 16-bit integer in big-endian byte order to a
 * (possibly) unaligned address.
 */
static inline void su16be(u8 *dst, u16 val);

static inline i32 li32be_aligned(const i32 *src);
static inline u32 lu32be_aligned(const u32 *src);
static inline i32 li32be(const u8 *src);
static inline u32 lu32be(const u8 *src);

static inline void si32be_aligned(i32 *dst, i32 val);
static inline void su32be_aligned(u32 *dst, u32 val);
static inline void si32be(u8 *dst, i32 val);
static inline void su32be(u8 *dst, u32 val);

static inline i64 li64be_aligned(const i64 *src);
static inline u64 lu64be_aligned(const u64 *src);
static inline i64 li64be(const u8 *src);
static inline u64 lu64be(const u8 *src);

static inline void si64be_aligned(i64 *dst, i64 val);
static inline void su64be_aligned(u64 *dst, u64 val);
static inline void si64be(u8 *dst, i64 val);
static inline void su64be(u8 *dst, u64 val);

static inline f32 lf32be(const u8 *src);
static inline f32 lf32be_aligned(const f32 *src);
static inline void sf32be(u8 *dst, f32 val);
static inline void sf32be_aligned(f32 *dst, f32 val);

#define SN_LOAD_BE16(Src, Type)                  \
  const u8 *src8 = (const u8 *)Src;              \
  Type ret = (Type)((src8[0] << 8) | (src8[1])); \
  return ret;

static inline i16 li16be_aligned(const i16 *src) {
  SN_LOAD_BE16(src, i16);
}

static inline u16 lu16be_aligned(const u16 *src) {
  SN_LOAD_BE16(src, u16);
}

static inline i16 li16be(const u8 *src) {
  SN_LOAD_BE16(src, i16);
}

static inline u16 lu16be(const u8 *src) {
  SN_LOAD_BE16(src, u16);
}

#define SN_STORE_BE16(Dst, Val)    \
  u8 *dst8 = (u8 *)Dst;            \
  dst8[0] = (u8)(Val >> 8) & 0xFF; \
  dst8[1] = (u8)(Val >> 0) & 0xFF;

static inline void si16be_aligned(i16 *dst, i16 val) {
  SN_STORE_BE16(dst, val);
}

static inline void su16be_aligned(u16 *dst, u16 val) {
  SN_STORE_BE16(dst, val);
}

static inline void si16be(u8 *dst, i16 val) {
  SN_STORE_BE16(dst, val);
}

static inline void su16be(u8 *dst, u16 val) {
  SN_STORE_BE16(dst, val);
}

#define SN_LOAD_BE32(Src, Type)                                          \
  const u8 *src8 = (const u8 *)Src;                                      \
  Type ret = (Type)((src8[0] << 24) | (src8[1] << 16) | (src8[2] << 8) | \
                    (src8[3] << 0));                                     \
  return ret;

static inline i32 li32be_aligned(const i32 *src) {
  SN_LOAD_BE32(src, i32);
}

static inline u32 lu32be_aligned(const u32 *src) {
  SN_LOAD_BE32(src, u32);
}

static inline i32 li32be(const u8 *src) {
  SN_LOAD_BE32(src, i32);
}

static inline u32 lu32be(const u8 *src) {
  SN_LOAD_BE32(src, u32);
}

#define SN_STORE_BE32(Dst, Val)     \
  u8 *dst8 = (u8 *)Dst;             \
  dst8[0] = (u8)(Val >> 24) & 0xFF; \
  dst8[1] = (u8)(Val >> 16) & 0xFF; \
  dst8[2] = (u8)(Val >> 8) & 0xFF;  \
  dst8[3] = (u8)(Val >> 0) & 0xFF;

static inline void si32be_aligned(i32 *dst, i32 val) {
  SN_STORE_BE32(dst, val);
}

static inline void su32be_aligned(u32 *dst, u32 val) {
  SN_STORE_BE32(dst, val);
}

static inline void si32be(u8 *dst, i32 val) {
  SN_STORE_BE32(dst, val);
}

static inline void su32be(u8 *dst, u32 val) {
  SN_STORE_BE32(dst, val);
}

#define SN_LOAD_BYTE_SHIFT64(Src8, Idx, Shift) (((u64)(Src8[Idx])) << Shift)

#define SN_LOAD_BE64(Src, Type)                         \
  const u8 *src8 = (const u8 *)Src;                     \
  Type ret = (Type)(SN_LOAD_BYTE_SHIFT64(src8, 0, 56) | \
                    SN_LOAD_BYTE_SHIFT64(src8, 1, 48) | \
                    SN_LOAD_BYTE_SHIFT64(src8, 2, 40) | \
                    SN_LOAD_BYTE_SHIFT64(src8, 3, 32) | \
                    SN_LOAD_BYTE_SHIFT64(src8, 4, 24) | \
                    SN_LOAD_BYTE_SHIFT64(src8, 5, 16) | \
                    SN_LOAD_BYTE_SHIFT64(src8, 6, 8) |  \
                    SN_LOAD_BYTE_SHIFT64(src8, 7, 0));  \
  return ret;

static inline i64 li64be_aligned(const i64 *src) {
  SN_LOAD_BE64(src, i64);
}

static inline u64 lu64be_aligned(const u64 *src) {
  SN_LOAD_BE64(src, u64);
}

static inline i64 li64be(const u8 *src) {
  SN_LOAD_BE64(src, i64);
}

static inline u64 lu64be(const u8 *src) {
  SN_LOAD_BE64(src, u64);
}

#define SN_STORE_BE64(Dst, Val)     \
  u8 *dst8 = (u8 *)Dst;             \
  dst8[0] = (u8)(Val >> 56) & 0xFF; \
  dst8[1] = (u8)(Val >> 48) & 0xFF; \
  dst8[2] = (u8)(Val >> 40) & 0xFF; \
  dst8[3] = (u8)(Val >> 32) & 0xFF; \
  dst8[4] = (u8)(Val >> 24) & 0xFF; \
  dst8[5] = (u8)(Val >> 16) & 0xFF; \
  dst8[6] = (u8)(Val >> 8) & 0xFF;  \
  dst8[7] = (u8)(Val >> 0) & 0xFF;

static inline void si64be_aligned(i64 *dst, i64 val) {
  SN_STORE_BE64(dst, val);
}

static inline void su64be_aligned(u64 *dst, u64 val) {
  SN_STORE_BE64(dst, val);
}

static inline void si64be(u8 *dst, i64 val) {
  SN_STORE_BE64(dst, val);
}

static inline void su64be(u8 *dst, u64 val) {
  SN_STORE_BE64(dst, val);
}

static inline f32 lf32be(const u8 *src) {
  u32 tmp = lu32be(src);
  f32 ret;
  memcpy(&ret, &tmp, sizeof(ret));
  return ret;
}

static inline f32 lf32be_aligned(const f32 *src) {
  u32 tmp = lu32be_aligned((const u32 *)src);
  f32 ret;
  memcpy(&ret, &tmp, sizeof(ret));
  return ret;
}

static inline void sf32be(u8 *dst, f32 val) {
  u32 tmp;
  memcpy(&tmp, &val, sizeof(tmp));
  su32be(dst, tmp);
}

static inline void sf32be_aligned(f32 *dst, f32 val) {
  u32 tmp;
  memcpy(&tmp, &val, sizeof(tmp));
  su32be_aligned((u32 *)dst, tmp);
}

#if __cplusplus
}
#endif