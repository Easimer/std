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

SN_TEST(MergeSort_i32, sortSucceeds) {
  i32 keys[4] = {1327110793, 1899266835, 702222870, 830235625};
  i32 out[4];
  mergeSort(sliceFrom(out), sliceFrom(keys));

  for (u32 i = 0; i < 3; i++) {
    CHECK(out[i] < out[i + 1]);
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
