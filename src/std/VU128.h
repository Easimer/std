#pragma once

#include "std/Types.h"

#if _MSC_VER
#define SN_STD_OUT_WRITES_TO(Size, Count) _Out_writes_to_(Size, Count)
#else
#define SN_STD_OUT_WRITES_TO(Size, Count)
#endif

#if __cplusplus
extern "C" {
#endif

/**
 * \brief Encodes a 32-bit unsigned integer VU128 and writes it to the provided
 * buffer.
 *
 * \param buf A valid pointer to the destination buffer
 * \param x Value to be encoded
 * \returns The length of the encoded value in `buf`
 */
u32 vu128_encode(SN_STD_OUT_WRITES_TO(5, return ) u8 buf[5], const u32 x);

/**
 * \brief Begins decoding a 32-bit unsigned value encoded using VU128.
 *
 * \param buf A pointer to the first byte of a VU128-encoded value.
 * \returns The number of *additional* bytes that have to be read by the
 * caller.
 */
u32 vu128_begin_decode(const u8 buf[1]);

/**
 * \brief Decodes a VU128-encoded value.
 * \returns The decoded value
 */
u32 vu128_decode(const u8 buf[5]);

#if __cplusplus
}
#endif
