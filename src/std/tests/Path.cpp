#include <std/Check.h>
#include <std/Path.hpp>
#include <std/Testing.hpp>
#include "std/SliceUtils.hpp"

#include <std/log.h>

SN_TEST(Path, dirnamePathToFile) {
  Slice<const char> res = dirname(sliceFromConstChar("dir/file"));
  CHECK(res == Slice<const char> sliceFromConstChar("dir"));
}

SN_TEST(Path, dirnameRootFile) {
  Slice<const char> res = dirname(sliceFromConstChar("/file"));
  CHECK(res == Slice<const char> sliceFromConstChar("/"));
}

SN_TEST(Path, dirnameRelative) {
  Slice<const char> res = dirname(sliceFromConstChar("./file"));
  CHECK(res == Slice<const char> sliceFromConstChar("."));
}

SN_TEST(Path, dirnameFilename) {
  Slice<const char> res = dirname(sliceFromConstChar("file"));
  CHECK(res == Slice<const char> sliceFromConstChar("."));
}

SN_TEST(Path, dirnameEmpty) {
  Slice<const char> res = dirname(sliceFromConstChar(""));
  CHECK(res == Slice<const char> sliceFromConstChar("."));
}

SN_TEST(Path, dirnameComplex) {
  Slice<const char> res = dirname(sliceFromConstChar("/dir/asd/file"));
  CHECK(res == Slice<const char> sliceFromConstChar("/dir/asd"));
}

SN_TEST(Path, dirnameNoFilename) {
  Slice<const char> res = dirname(sliceFromConstChar("/dir/asd/"));
  CHECK(res == Slice<const char> sliceFromConstChar("/dir"));
}

SN_TEST(Path, basenamePathToFile) {
  Slice<const char> res = basename(sliceFromConstChar("dir/file"));
  CHECK(res == Slice<const char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameRootFile) {
  Slice<const char> res = basename(sliceFromConstChar("/file"));
  CHECK(res == Slice<const char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameRelative) {
  Slice<const char> res = basename(sliceFromConstChar("./file"));
  CHECK(res == Slice<const char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameFilename) {
  Slice<const char> res = basename(sliceFromConstChar("file"));
  CHECK(res == Slice<const char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameEmpty) {
  Slice<const char> res = basename(sliceFromConstChar(""));
  log_info("'%.*s'", FMT_SLICE(res));
  CHECK(res.empty());
}

SN_TEST(Path, basenameComplex) {
  Slice<const char> res = basename(sliceFromConstChar("/dir/asd/file"));
  CHECK(res == Slice<const char> sliceFromConstChar("file"));
}

SN_TEST(Path, basenameNoFilename) {
  Slice<const char> res = basename(sliceFromConstChar("/dir/asd/"));
  CHECK(res == Slice<const char> sliceFromConstChar("asd"));
}
