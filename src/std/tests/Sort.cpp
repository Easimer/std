#include <std/Check.h>
#include <std/Slice.hpp>
#include <std/RadixSort.hpp>
#include <std/Sort.hpp>
#include <std/Testing.hpp>

SN_TEST(RadixSort_u32, emptySortSucceeds) {
  radixSort(0, nullptr, nullptr, (const u32*)nullptr);
}

SN_TEST(RadixSort_u32, alreadySorted) {
  u32 indices[4] = {0, 1, 2, 3};
  u32 keys[4] = {813570146U, 1749442322U, 3102860508U, 3726812458U};
  u32 temp[4];
  const u32 indicesExpected[4] = {0, 1, 2, 3};
  radixSort(4, indices, temp, keys);

  for (u32 i = 0; i < 4; i++) {
    CHECK(indices[i] == indicesExpected[i]);
  }
}

SN_TEST(RadixSort_u32, sortSucceeds) {
  u32 indices[4] = {0, 1, 2, 3};
  u32 keys[4] = {3102860508, 1749442322, 813570146, 3726812458};
  u32 temp[4];
  const u32 indicesExpected[4] = {2, 1, 0, 3};
  radixSort(4, indices, temp, keys);

  for (u32 i = 0; i < 4; i++) {
    CHECK(indices[i] == indicesExpected[i]);
  }
}

SN_TEST(RadixSort_i32, sortSucceeds) {
  u32 indices[4] = {0, 1, 2, 3};
  i32 keys[4] = {1327110793, 1899266835, 702222870, 830235625};
  u32 temp[4];
  const u32 indicesExpected[4] = {2, 3, 0, 1};
  radixSort(4, indices, temp, keys);

  for (u32 i = 0; i < 4; i++) {
    CHECK(indices[i] == indicesExpected[i]);
  }
}

SN_TEST(RadixSort_u64, sortSucceeds) {
  u32 indices[4] = {0, 1, 2, 3};
  u64 keys[4] = {5264794389990322948ULL, 8773299985955849259ULL,
                 18026563674236274730ULL, 4287803270097231623ULL};
  u32 temp[4];
  const u32 indicesExpected[4] = {3, 0, 1, 2};
  radixSort(4, indices, temp, keys);

  for (u32 i = 0; i < 4; i++) {
    CHECK(indices[i] == indicesExpected[i]);
  }
}

SN_TEST(MergeSort, emptySortSucceeds) {
  Slice<u32> src, dst;
  mergeSort(dst, src);
}

SN_TEST(MergeSort_u32, sortSucceeds) {
  u32 keys[4] = {3102860508, 1749442322, 813570146, 3726812458};
  u32 out[4];
  mergeSort(sliceFrom(out), sliceFrom(keys));

  for (u32 i = 0; i < 3; i++) {
    CHECK(out[i] < out[i + 1]);
  }
}

SN_TEST(MergeSort_u32, inplaceSortSucceeds) {
  u32 keys[4] = {3102860508, 1749442322, 813570146, 3726812458};
  mergeSort(sliceFrom(keys));

  for (u32 i = 0; i < 3; i++) {
    CHECK(keys[i] < keys[i + 1]);
  }
}

SN_TEST(MergeSort_i32, sortSucceeds) {
  i32 keys[4] = {1327110793, 1899266835, 702222870, 830235625};
  i32 out[4];
  mergeSort(sliceFrom(out), sliceFrom(keys));

  for (u32 i = 0; i < 3; i++) {
    CHECK(out[i] < out[i + 1]);
  }
}

SN_TEST(MergeSort_i32, sortWithCmpSucceeds) {
  i32 keys[4] = {1327110793, 1899266835, 702222870, 830235625};
  i32 out[4];
  mergeSort(sliceFrom(out), sliceFrom(keys),
            [](i32 l, i32 r) { return l < r; });

  for (u32 i = 0; i < 3; i++) {
    CHECK(out[i] < out[i + 1]);
  }
}

SN_TEST(MergeSort_i32, sortWithCmpDescendingSucceeds) {
  i32 keys[4] = {1327110793, 1899266835, 702222870, 830235625};
  i32 out[4];
  mergeSort(sliceFrom(out), sliceFrom(keys),
            [](i32 l, i32 r) { return l > r; });

  for (u32 i = 0; i < 3; i++) {
    CHECK(out[i] > out[i + 1]);
  }
}

SN_TEST(MergeSort_u64, sortSucceeds) {
  u64 keys[4] = {5264794389990322948ULL, 8773299985955849259ULL,
                 18026563674236274730ULL, 4287803270097231623ULL};
  u64 out[4];
  mergeSort(sliceFrom(out), sliceFrom(keys));

  for (u32 i = 0; i < 3; i++) {
    CHECK(out[i] < out[i + 1]);
  }
}

SN_TEST(MergeSort_u32, sortOdd) {
  u32 keys[5] = {5, 4, 3, 2, 1};
  u32 out[5];
  mergeSort(sliceFrom(out), sliceFrom(keys));

  CHECK(out[0] == 1);
  CHECK(out[1] == 2);
  CHECK(out[2] == 3);
  CHECK(out[3] == 4);
  CHECK(out[4] == 5);
}

SN_TEST(MergeSort_u32, sortOne) {
  u32 keys[1] = {1};
  u32 out[1];
  mergeSort(sliceFrom(out), sliceFrom(keys));

  CHECK(out[0] == 1);
}

auto abs = [](i32 x) { return x > 0 ? x : -x; };
struct Comparable {
  i32 x, y;

  i32 length() const { return abs(x) + abs(y); }

  bool operator<(const Comparable &other) const {
    return length() < other.length();
  }
};

SN_TEST(MergeSort_ComparableStruct, sortSucceeds) {
  Comparable elems[4] = {
      {7, 3},
      {9, 10},
      {1, -1},
      {4, 5},
  };
  mergeSort(sliceFrom(elems));

  for (u32 i = 0; i < 3; i++) {
    CHECK(elems[i] < elems[i + 1]);
  }
}

SN_TEST(MergeSort_ComparableStruct, customCmpSortSucceeds) {
  Comparable elems[4] = {
      {7, 3},
      {9, 10},
      {1, -1},
      {4, 5},
  };
  mergeSort(sliceFrom(elems),
            [](const Comparable &l, const Comparable &r) { return r < l; });

  for (u32 i = 0; i < 3; i++) {
    CHECK(elems[i + 1] < elems[i]);
  }
}

SN_TEST(MergeSort_ComparableStruct, customCmpByConstRefSucceeds) {
  Comparable elems[4] = {
      {7, 3},
      {9, 10},
      {1, -1},
      {4, 5},
  };

  struct CustomCmp {
    bool isDescending = false;
    bool operator()(const Comparable &l, const Comparable &r) const noexcept {
      if (isDescending) {
        return r < l;
      } else {
        return l < r;
      }
    }
  };

  CustomCmp cmp;
  mergeSort(sliceFrom(elems), cmp);
  for (u32 i = 0; i < 3; i++) {
    CHECK(elems[i] < elems[i + 1]);
  }

  cmp.isDescending = true;
  mergeSort(sliceFrom(elems), cmp);
  for (u32 i = 0; i < 3; i++) {
    CHECK(elems[i + 1] < elems[i]);
  }
}
