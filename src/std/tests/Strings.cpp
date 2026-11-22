#include "std/Strings.hpp"
#include "std/Testing.hpp"

SN_TEST(Strings, split_empty) {
  Arena::Scope temp = Arena::getScratch();

  Slice<char> empty;
  Slice<Slice<char>> res = split(temp, empty, ',');
  CHECK(res.empty());
}

SN_TEST(Strings, split_normal) {
  Arena::Scope temp = Arena::getScratch();

  Slice<char> input = "asd,123,wawa"_slice;
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0] == "asd"_slice);
  CHECK(res[1] == "123"_slice);
  CHECK(res[2] == "wawa"_slice);
}

SN_TEST(Strings, split_leadingSeparator) {
  Arena::Scope temp = Arena::getScratch();

  Slice<char> input = ",123,wawa"_slice;
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0].empty());
  CHECK(res[1] == "123"_slice);
  CHECK(res[2] == "wawa"_slice);
}

SN_TEST(Strings, split_trailingSeparator) {
  Arena::Scope temp = Arena::getScratch();

  Slice<char> input = "asd,123,"_slice;
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0] == "asd"_slice);
  CHECK(res[1] == "123"_slice);
  CHECK(res[2].empty());
}

SN_TEST(Strings, split_emptyMiddle) {
  Arena::Scope temp = Arena::getScratch();

  Slice<char> input = "asd,,wawa"_slice;
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0] == "asd"_slice);
  CHECK(res[1].empty());
  CHECK(res[2] == "wawa"_slice);
}

SN_TEST(Strings, split_singleSeparator) {
  Arena::Scope temp = Arena::getScratch();

  Slice<char> input = ","_slice;
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 2);
  CHECK(res[0].empty());
  CHECK(res[1].empty());
}
