#pragma once

#include "std/Array.hpp"
#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"
#include "std/Types.h"

inline static u32 _log2i(u64 x) {
  return 8 * sizeof(unsigned long long) - countLeadingZeros64(x) - 1;
}

/**
 * \brief A segment-based dynamic array of T.
 *
 * Elements are stored in power-of-two sized segments allocated on-demand as the
 * array grows. The first six segments (sized 1,2,...,32) are skipped, so
 * segment 0 stores 64 elements.
 */
template <typename T>
struct SegmentArray {
  static constexpr u32 SMALL_SEGMENTS_TO_SKIP = 6;

  Arena *arena;
  /** \brief Number of valid elements in the array */
  u32 length;

  u32 numSegments;
  Array<T *, 26> arrSegments;

  constexpr SegmentArray() : arena(nullptr), length(0), numSegments(0) {}
  constexpr SegmentArray(Arena *arena)
      : arena(arena), length(0), numSegments(0) {}

  T &operator[](u32 i) { return *getSlotForItem(i); }
  const T &operator[](u32 i) const { return *getSlotForItem(i); }

  T &push() {
    growIfNeededPrePush();

    T *slot = getSlotForItem(length);
    *slot = {};
    length += 1;
    return *slot;
  }

  T &push(const T &value) {
    growIfNeededPrePush();

    T *slot = getSlotForItem(length);
    *slot = value;
    length += 1;
    return *slot;
  }

  void push(Slice<const T> elements) {
    while (!elements.empty()) {
      growIfNeededPrePush();

      u32 idxLastSegment = numSegments - 1;
      Slice<T> freeSlots = sliceOfSegmentFreeHalf(idxLastSegment);
      Slice<const T> src = elements.subarray(0, freeSlots.length);
      copy(freeSlots, src);

      elements.shrinkFromLeftByCount(src.length);
      length += src.length;
    }
  }

  static constexpr u32 sizeOfSegment(u32 idxSegment) {
    // 2**(SSTS + idxSegment)
    return (1 << SMALL_SEGMENTS_TO_SKIP) << idxSegment;
  }

  static constexpr u32 capacityForSegmentCount(u32 numSegments) {
    return sizeOfSegment(numSegments) - sizeOfSegment(0);
  }

  static u32 getSegmentForItem(u32 idxItem) {
    return _log2i((idxItem >> SMALL_SEGMENTS_TO_SKIP) + 1);
  }

  T *getSlotForItem(u32 idxItem) const {
    u32 idxSegment = getSegmentForItem(idxItem);
    u32 idxSlot = idxItem - capacityForSegmentCount(idxSegment);
    T *segment = arrSegments[idxSegment];
    DCHECK(segment != nullptr);
    return &segment[idxSlot];
  }

  /**
   * \brief Grows the array by a new segment
   */
  void grow() {
    u32 idxNewSegment = numSegments;
    arrSegments[idxNewSegment] = alloc<T>(arena, sizeOfSegment(idxNewSegment));
    numSegments += 1;
  }

  /**
   * \brief Grows the array if there is no space for one more element to be
   * pushed
   */
  void growIfNeededPrePush() {
    if (length >= capacityForSegmentCount(numSegments)) {
      grow();
    }
  }

  /**
   * \brief Returns a slice on the specified segment.
   */
  Slice<T> sliceOfSegment(u32 idxSegment) {
    T *base = arrSegments[idxSegment];
    if (base == nullptr) {
      return {};
    }

    return {base, sizeOfSegment(idxSegment)};
  }

  /**
   * \brief Returns a slice on the free/upper half of the specified segment.
   */
  Slice<T> sliceOfSegmentFreeHalf(u32 idxSegment) {
    DCHECK(idxSegment < numSegments);
    u32 cap = capacityForSegmentCount(idxSegment + 1);
    if (length >= cap) {
      // This segment is full
      return {};
    }

    u32 numFreeSlots = cap - length;
    Slice<T> ret = sliceOfSegment(idxSegment);
    shrinkFromLeftByCount(&ret, ret.length - numFreeSlots);
    return ret;
  }
};

/**
 * \brief Copies the contents of the segment array into a new linear array
 * allocated into the specified arena.
 */
template <typename T>
Slice<T> copyToSlice(Arena *arena, const SegmentArray<T> &sa) {
  const u32 numElems = sa.length;

  if (numElems == 0) {
    return {};
  }

  Slice<T> ret;
  alloc(arena, numElems, ret);

  u32 idxElem = 0;
  const u32 numSegments = sa.numSegments;

  for (u32 idxSegment = 0; idxSegment < numSegments; idxSegment++) {
    const u32 lenSegment = sa.sizeOfSegment(idxSegment);
    T *const segment = sa.arrSegments[idxSegment];
    DCHECK(segment != nullptr);

    for (u32 idxSlot = 0; idxSlot < lenSegment && idxElem < numElems;
         idxSlot++) {
      ret[idxElem] = segment[idxSlot];
      idxElem += 1;
    }
  }

  DCHECK(idxElem == numElems);

  return ret;
}