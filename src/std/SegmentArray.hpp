#pragma once

#include "std/Array.hpp"
#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"
#include "std/Types.h"

inline static u32 _log2i(size_t x) {
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

  T &operator[](size_t i) { return *getSlotForItem(i); }
  const T &operator[](size_t i) const { return *getSlotForItem(i); }

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

  void push(Slice<T> elements) {
    while (!elements.empty()) {
      growIfNeededPrePush();

      u32 idxLastSegment = numSegments - 1;
      MutSlice<T> freeSlots = sliceOfSegmentFreeHalf(idxLastSegment);
      Slice<T> src = elements.subarray(0, freeSlots.length);
      freeSlots.copy(src);

      elements.shrinkFromLeftByCount(src.length);
      DCHECK(src.length <= 0xFFFFFFFF);
      length += (u32)src.length;
    }
  }

  static constexpr u32 sizeOfSegment(u32 idxSegment) {
    // 2**(SSTS + idxSegment)
    return (1 << SMALL_SEGMENTS_TO_SKIP) << idxSegment;
  }

  static constexpr size_t capacityForSegmentCount(u32 numSegments) {
    return sizeOfSegment(numSegments) - sizeOfSegment(0);
  }

  static u32 getSegmentForItem(size_t idxItem) {
    return _log2i((idxItem >> SMALL_SEGMENTS_TO_SKIP) + 1);
  }

  T *getSlotForItem(size_t idxItem) const {
    u32 idxSegment = getSegmentForItem(idxItem);
    size_t idxSlot = idxItem - capacityForSegmentCount(idxSegment);
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
  MutSlice<T> sliceOfSegment(u32 idxSegment) {
    T *base = arrSegments[idxSegment];
    if (base == nullptr) {
      return {};
    }

    return {base, sizeOfSegment(idxSegment)};
  }

  /**
   * \brief Returns a slice on the free/upper half of the specified segment.
   */
  MutSlice<T> sliceOfSegmentFreeHalf(u32 idxSegment) {
    DCHECK(idxSegment < numSegments);
    size_t cap = capacityForSegmentCount(idxSegment + 1);
    if (length >= cap) {
      // This segment is full
      return {};
    }

    size_t numFreeSlots = cap - length;
    MutSlice<T> ret = sliceOfSegment(idxSegment);
    ret.shrinkFromLeftByCount(ret.length - numFreeSlots);
    return ret;
  }
};

/**
 * \brief Copies the contents of the segment array into a new linear array
 * allocated into the specified arena.
 */
template <typename T>
MutSlice<T> copyToSlice(Arena *arena, const SegmentArray<T> &sa) {
  const size_t numElems = sa.length;

  if (numElems == 0) {
    return {};
  }

  MutSlice<T> ret;
  alloc(arena, numElems, ret);

  size_t idxElem = 0;
  const u32 numSegments = sa.numSegments;

  for (u32 idxSegment = 0; idxSegment < numSegments; idxSegment++) {
    const u32 lenSegment = sa.sizeOfSegment(idxSegment);
    T *const segment = sa.arrSegments[idxSegment];
    DCHECK(segment != nullptr);

    for (size_t idxSlot = 0; idxSlot < lenSegment && idxElem < numElems;
         idxSlot++) {
      ret[idxElem] = segment[idxSlot];
      idxElem += 1;
    }
  }

  DCHECK(idxElem == numElems);

  return ret;
}