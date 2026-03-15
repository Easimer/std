#include <std/Check.h>
#include <std/Slice.hpp>
#include <std/Array.hpp>
#include <std/Testing.hpp>

SN_TEST(Array, initializerList) {
  Array<u32, 4> arr = {1, 2, 4, 5};
}

SN_TEST(Array, indexing) {
  const Array<u32, 4> arr = {1, 2, 4, 5};
  CHECK(arr[0] == 1);
  CHECK(arr[1] == 2);
  CHECK(arr[2] == 4);
  CHECK(arr[3] == 5);
}

SN_TEST(Array, indexingStore) {
  Array<u32, 4> arr = {1, 2, 4, 5};

  CHECK(arr[2] == 4);
  CHECK(arr.data[2] == 4);

  arr[2] = 9;

  CHECK(arr[2] == 9);
  CHECK(arr.data[2] == 9);
}

SN_TEST(Array, implicitConversionToSlice) {
  Array<u32, 4> arr = {1, 2, 4, 5};

  Slice<u32> view = arr;
  CHECK(view.data == arr.data);
  CHECK(view.length == 4);
}

SN_TEST(Array, implicitConversionToConstSlice) {
  Array<u32, 4> arr = {1, 2, 4, 5};

  Slice<const u32> view = arr;
  CHECK(view.data == arr.data);
  CHECK(view.length == 4);
}

SN_TEST(Array, constImplicitConversionToConstSlice) {
  const Array<u32, 4> arr = {1, 2, 4, 5};

  Slice<const u32> view = arr;
  CHECK(view.data == arr.data);
  CHECK(view.length == 4);
}

SN_TEST(Array, subarray) {
  Array<u32, 4> arr = {1, 2, 4, 5};
  Slice<u32> view = arr.subarray(0, 3);
  CHECK(view.data == arr.data);
  CHECK(view.length == 3);
}

SN_TEST(Array, subarrayOffset) {
  Array<u32, 4> arr = {1, 2, 4, 5};
  Slice<u32> view = arr.subarray(1, 4);
  CHECK(view.data == arr.data + 1);
  CHECK(view.length == 3);
}
