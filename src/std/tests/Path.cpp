#include <std/Check.h>
#include <std/Path.hpp>
#include <std/Testing.hpp>
#include "std/SliceUtils.hpp"

#include <std/log.h>

SN_TEST(Path, dirnamePathToFile) {
  Slice<char> res = dirname(sliceFromConstChar("dir/file"));
  CHECK(res == Slice<char> sliceFromConstChar("dir"));
}

SN_TEST(Path, dirnameRootFile) {
  Slice<char> res = dirname(sliceFromConstChar("/file"));
  CHECK(res == Slice<char> sliceFromConstChar("/"));
}

SN_TEST(Path, dirnameRelative) {
  Slice<char> res = dirname(sliceFromConstChar("./file"));
  CHECK(res == Slice<char> sliceFromConstChar("."));
}

SN_TEST(Path, dirnameFilename) {
  Slice<char> res = dirname(sliceFromConstChar("file"));
  CHECK(res == Slice<char> sliceFromConstChar("."));
}

SN_TEST(Path, dirnameEmpty) {
  Slice<char> res = dirname(sliceFromConstChar(""));
  CHECK(res == Slice<char> sliceFromConstChar("."));
}

SN_TEST(Path, dirnameComplex) {
  Slice<char> res = dirname(sliceFromConstChar("/dir/asd/file"));
  CHECK(res == Slice<char> sliceFromConstChar("/dir/asd"));
}

SN_TEST(Path, dirnameNoFilename) {
  Slice<char> res = dirname(sliceFromConstChar("/dir/asd/"));
  CHECK(res == Slice<char> sliceFromConstChar("/dir"));
}

SN_TEST(Path, basenamePathToFile) {
  Slice<char> res = basename(sliceFromConstChar("dir/file"));
  CHECK(res == Slice<char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameRootFile) {
  Slice<char> res = basename(sliceFromConstChar("/file"));
  CHECK(res == Slice<char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameRelative) {
  Slice<char> res = basename(sliceFromConstChar("./file"));
  CHECK(res == Slice<char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameFilename) {
  Slice<char> res = basename(sliceFromConstChar("file"));
  CHECK(res == Slice<char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameEmpty) {
  Slice<char> res = basename(sliceFromConstChar(""));
  log_info("'%.*s'", FMT_SLICE(res));
  CHECK(res.empty());
}

SN_TEST(Path, basenameComplex) {
  Slice<char> res = basename(sliceFromConstChar("/dir/asd/file"));
  CHECK(res == Slice<char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameNoFilename) {
  Slice<char> res = basename(sliceFromConstChar("/dir/asd/"));
  CHECK(res == Slice<char> sliceFromConstChar("asd"));
}
