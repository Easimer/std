#include "std/Strings.hpp"
#include "std/Testing.hpp"

SN_TEST(Strings, split_empty) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Slice<char> empty;
  Slice<Slice<char>> res = split(temp, empty, ',');
  CHECK(res.empty());
}

SN_TEST(Strings, split_normal) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Slice<char> input = sliceFromConstChar("asd,123,wawa");
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0] == Slice<char>(sliceFromConstChar("asd")));
  CHECK(res[1] == Slice<char>(sliceFromConstChar("123")));
  CHECK(res[2] == Slice<char>(sliceFromConstChar("wawa")));
}

SN_TEST(Strings, split_leadingSeparator) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Slice<char> input = sliceFromConstChar(",123,wawa");
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0].empty());
  CHECK(res[1] == Slice<char>(sliceFromConstChar("123")));
  CHECK(res[2] == Slice<char>(sliceFromConstChar("wawa")));
}

SN_TEST(Strings, split_trailingSeparator) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Slice<char> input = sliceFromConstChar("asd,123,");
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0] == Slice<char>(sliceFromConstChar("asd")));
  CHECK(res[1] == Slice<char>(sliceFromConstChar("123")));
  CHECK(res[2].empty());
}

SN_TEST(Strings, split_emptyMiddle) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Slice<char> input = sliceFromConstChar("asd,,wawa");
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 3);
  CHECK(res[0] == Slice<char>(sliceFromConstChar("asd")));
  CHECK(res[1].empty());
  CHECK(res[2] == Slice<char>(sliceFromConstChar("wawa")));
}

SN_TEST(Strings, split_singleSeparator) {
  Arena::Scope temp = getScratch(nullptr, 0);

  Slice<char> input = sliceFromConstChar(",");
  Slice<Slice<char>> res = split(temp, input, ',');
  CHECK(res.length == 2);
  CHECK(res[0].empty());
  CHECK(res[1].empty());
}