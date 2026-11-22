#include <std/Check.h>
#include <std/Path.hpp>
#include <std/Testing.hpp>
#include "std/SliceUtils.hpp"

#include <std/log.h>

SN_TEST(Path, dirnamePathToFile) {
  Slice<char> res = dirname("dir/file"_slice);
  CHECK(res == "dir"_slice);
}

SN_TEST(Path, dirnameRootFile) {
  Slice<char> res = dirname("/file"_slice);
  CHECK(res == "/"_slice);
}

SN_TEST(Path, dirnameRelative) {
  Slice<char> res = dirname("./file"_slice);
  CHECK(res == "."_slice);
}

SN_TEST(Path, dirnameFilename) {
  Slice<char> res = dirname("file"_slice);
  CHECK(res == "."_slice);
}

SN_TEST(Path, dirnameEmpty) {
  Slice<char> res = dirname(""_slice);
  CHECK(res == "."_slice);
}

SN_TEST(Path, dirnameComplex) {
  Slice<char> res = dirname("/dir/asd/file"_slice);
  CHECK(res == "/dir/asd"_slice);
}

SN_TEST(Path, dirnameNoFilename) {
  Slice<char> res = dirname("/dir/asd/"_slice);
  CHECK(res == "/dir"_slice);
}

SN_TEST(Path, basenamePathToFile) {
  Slice<char> res = basename("dir/file"_slice);
  CHECK(res == "file"_slice);
}

SN_TEST(Path, basenameRootFile) {
  Slice<char> res = basename("/file"_slice);
  CHECK(res == "file"_slice);
}

SN_TEST(Path, basenameRelative) {
  Slice<char> res = basename("./file"_slice);
  CHECK(res == "file"_slice);
}

SN_TEST(Path, basenameFilename) {
  Slice<char> res = basename("file"_slice);
  CHECK(res == "file"_slice);
}

SN_TEST(Path, basenameEmpty) {
  Slice<char> res = basename(""_slice);
  log_info("'%.*s'", FMT_SLICE(res));
  CHECK(res.empty());
}

SN_TEST(Path, basenameComplex) {
  Slice<char> res = basename("/dir/asd/file"_slice);
  CHECK(res == "file"_slice);
}

SN_TEST(Path, basenameNoFilename) {
  Slice<char> res = basename("/dir/asd/"_slice);
  CHECK(res == "asd"_slice);
}
