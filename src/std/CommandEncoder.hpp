#pragma once

#include "std/Array.hpp"
#include "std/SegmentArray.hpp"
#include "std/Slice.hpp"
#include "std/Types.h"

/** @file CommandEncoder.hpp */

/**
 * \defgroup DeltaEncoding Delta encoding
 * @{
 */

/**
 * \brief A delta encoder for commands with multiple fields.
 *
 * \tparam FieldEnum An enum type with enumerations for every independent
 * command field.
 * 
 * \see CommandDecoder
 */
template <typename FieldEnum>
struct CommandEncoder {
  SegmentArray<u8> _buffer;
  u32 _dirtyMask = 0;

  /**
   * \brief Copies the encoded byte stream to a buffer allocated into the
   * specified arena. This buffer can be passed to a CommandDecoder.
   */
  Slice<const u8> extractBuffer(Arena *arena) {
    return copyToSlice(arena, _buffer).asConst();
  }

 protected:
  CommandEncoder(Arena *arena) : _buffer(arena) {}

  /**
   * @{
   * @name Command mutation
   * These functions mutate the fields of the current command and record into a
   * bitfield which of the fields have changed since the previous command.
   */

  template <FieldEnum DF>
  void set(u32 &field, u32 newValue) {
    u32 mask = (1u << DF);
    if (newValue != field) {
      field = newValue;
      _dirtyMask |= mask;
    }
  }

  /**
   * \brief Compares the new value to the field's current value; if they're not
   * equal, the field's flag in the dirty mask gets set and the field is
   * overwritten.
   * \param field Reference to a field in the state
   * \param value New value
   */
  template <FieldEnum DF, typename T>
  void set(T &field, const T &value) {
    u32 mask = (1u << DF);
    if (memcmp(&value, &field, sizeof(T)) != 0) {
      field = value;
      _dirtyMask |= mask;
    }
  }

  /**
   * \brief Compares the new value to the field's current value at the specified
   * index; if they're not equal, the subfield's flag in the dirty mask gets set
   * and the field is overwritten.
   *
   * \param field Reference to a field in the state
   * \param value New value
   *
   * \section flags Field flags
   * The subfield's flag is `1 << (DF + idx)`. So if `field` is an array of N
   * elements, then `FieldEnum` should have N enumerations for the N elements of
   * the array and `DF` should be the enum associated with the first element:
   * ```cpp
   * enum FieldEnum {
   * // ...
   *   FE_ARRAY_OF_THINGS0,
   *   FE_ARRAY_OF_THINGS1,
   *   FE_ARRAY_OF_THINGS2,
   * // ...
   *   FE_ARRAY_OF_THINGSN,
   * // ...
   * };
   *
   * struct State {
   * // ...
   *   Array<Thing, N> arrThings;
   * // ...
   * };
   *
   * encoder.set<FE_ARRAY_OF_THINGS0>(state.arrThings, idx, newValue);
   * ```
   */
  template <FieldEnum DF, size_t N, typename T>
  void set(Array<T, N> &field, u32 idx, const T &value) {
    DCHECK(idx < N);
    u32 mask = (1u << (DF + idx));
    if (value != field[idx]) {
      field[idx] = value;
      _dirtyMask |= mask;
    }
  }
  /**@}*/

  /**
   * @{
   * @name Encode changes
   * You can use these methods to build a function that writes the accumulated
   * changes into the output buffer.
   */

  /**
   * \brief Push the dirty mask into the byte stream. This must be called
   * before any of the `pushIf*` methods are called.
   */
  void pushDirtyMask() noexcept { pushBytesOf(_dirtyMask); }

  /**
   * \brief Pushes the specified value into the byte stream **if** the LSB of
   * the dirty mask is set. The dirty mask is then right shifted by one.
   */
  template <typename T>
  void pushIfFlag(const T &val) noexcept {
    if (_dirtyMask & 1) {
      pushBytesOf(val);
    }
    _dirtyMask >>= 1;
  }

  /**
   * \brief Pushes the elements of the array into the byte stream. Each element
   * is considered an independent field of the command and so element `i` is
   * only written to the stream **if** the corresponding bit `i` - starting from
   * the LSB - is set. The dirty mask is right shifted after every push.
   */
  template <size_t N, typename T>
  void pushIfFlag(const Array<T, N> &arr) noexcept {
    for (u32 i = 0; i < N; i++) {
      pushIfFlag(arr[i]);
    }
  }

  /**
   * \brief Pushes the array into the byte stream. The array is considered a
   * single element and it's only pushed into the byte stream **if** the LSB of
   * the dirty mask is set. The dirty mask is then right shifted by one.
   */
  template <size_t N, typename T>
  void pushIfFlagWhole(const Array<T, N> &arr) noexcept {
    if (_dirtyMask & 1) {
      for (u32 i = 0; i < N; i++) {
        pushBytesOf(arr[i]);
      }
    }
    _dirtyMask >>= 1;
  }

  /**
   * \brief Pushes the bytes of the specified value into the output stream.
   */
  template <typename T>
  void pushBytesOf(const T &val) noexcept {
    Slice<const u8> bytes = cast<const u8, const T>({&val, 1});
    _buffer.push(bytes);
  }

  void clearDirtyMask() noexcept { _dirtyMask = 0; }
  /**@}*/
};

/**@}*/
