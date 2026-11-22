#include "std/Testing.hpp"

#include "std/json/Parser.hpp"
#include "std/json/Utils.hpp"
#include "std/json/Value.hpp"

#include <math.h>

SN_TEST(JsonParser, EmptyObject) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "{}"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Object);
  CHECK(res.object().empty());
}

SN_TEST(JsonParser, EmptyArray) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  CHECK(res.array().empty());
}

SN_TEST(JsonParser, OpenArray) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "["_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, OpenArrayNested) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[[[[[[[[]]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, OpenObject) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "{"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, OpenObjectNested) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "{\"k\": {\"k\": {\"k\": {\"k\": {"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, NumberArray) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[1, 2.0, 1e+8, -4, -5.0, -6.7e+2, 1e-8, -1e-8]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  Slice<JsonValue> elems = res.array();
  CHECK(elems.length == 8);

  for (auto [e, _] : elems) {
    CHECK(isNumeric(e));
  }

  CHECK(elems[0].number == 1);
  CHECK(elems[1].number == 2);
  CHECK(elems[2].number == 1e+8);
  CHECK(elems[3].number == -4);
  CHECK(elems[4].number == -5.0);
  CHECK(elems[5].number == -6.7e+2);
  CHECK(elems[6].number == 1e-8);
  CHECK(elems[7].number == -1e-8);
}

SN_TEST(JsonParser, EmptyArrayNewLine) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[\n]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  Slice<JsonValue> elems = res.array();
  CHECK(elems.empty());
}

SN_TEST(JsonParser, EmptyArrayWhitespace) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src =
      "     \n      \n \n\n \r\n  [    \n ]    \n\n"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  Slice<JsonValue> elems = res.array();
  CHECK(elems.empty());
}

SN_TEST(JsonParser, StringEmpty) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "\"\""_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::String);
  CHECK(res.string().empty());
}

SN_TEST(JsonParser, StringSimple) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "\"asd\""_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::String);
  CHECK(res.string() == Slice<char>("asd"_slice));
}

SN_TEST(JsonParser, StringCodepoint) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "\"\\u039B\""_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::String);

  const u8 uc[2] = {0xCE, 0x9B};
  Slice<u8> ucs = sliceFrom(uc);
  CHECK(res.string() == ucs.cast<char>());
}

SN_TEST(JsonParser, StringArray) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[\"asd\", \"\", \"\\u0391\"]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  Slice<JsonValue> elems = res.array();
  CHECK(elems.length == 3);

  for (auto [e, _] : elems) {
    CHECK(isString(e));
  }
}

SN_TEST(JsonParser, ObjectKeyValues) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = 
      "{\"num\": -1.5, \"str\": \"string\", \"arr\": [1,2], \"obj\": {}, "
      "\"nul\": null, \"false\": false, \"tru\": true}"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Object);
  Slice<JsonKeyValue> kvs = res.object();
  CHECK(kvs.length == 7);

  ASSERT_EQUAL(kvs[0].key, "num"_slice);
  CHECK(kvs[0].value.type == JsonType::Number);
  CHECK(kvs[0].value.number == -1.5);

  ASSERT_EQUAL(kvs[1].key, "str"_slice);
  CHECK(kvs[1].value.type == JsonType::String);
  ASSERT_EQUAL(kvs[1].value.string(), "string"_slice);

  ASSERT_EQUAL(kvs[2].key, "arr"_slice);
  CHECK(kvs[2].value.type == JsonType::Array);
  CHECK(kvs[2].value.array().length == 2);
  CHECK(kvs[2].value.array()[0].type == JsonType::Number);
  CHECK(kvs[2].value.array()[0].number == 1);
  CHECK(kvs[2].value.array()[1].type == JsonType::Number);
  CHECK(kvs[2].value.array()[1].number == 2);

  ASSERT_EQUAL(kvs[3].key, "obj"_slice);
  CHECK(kvs[3].value.type == JsonType::Object);
  CHECK(kvs[3].value.object().empty());

  ASSERT_EQUAL(kvs[4].key, "nul"_slice);
  CHECK(kvs[4].value.type == JsonType::Null);

  ASSERT_EQUAL(kvs[5].key, "false"_slice);
  CHECK(kvs[5].value.type == JsonType::False);

  ASSERT_EQUAL(kvs[6].key, "tru"_slice);
  CHECK(kvs[6].value.type == JsonType::True);
}

SN_TEST(JsonParser, StringUtf8) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char8_t> src8 = u8"[\"€𝄞\"]"_slice;
  Slice<char> src = src8.cast<char>();
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);

  CHECK(res.type == JsonType::Array);
  CHECK(res.array().length == 1);
  Slice<char8_t> expected = u8"€𝄞"_slice;
  CHECK(res.array()[0].string() == expected.cast<char>());
}

SN_TEST(JsonParser, ArrayDoubleComma) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[1,,2]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, ArrayDoubleClose) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[\"x\"]]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, ArrayIncomplete) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[\"x\""_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, NumberWithAlpha) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[1.2a-3]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, TrailingChars) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[\"\"],"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, ArrayExtraComma) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[\"\",]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, NumberWithNoFractionalDigits) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[-2.]"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, StringStartEscapeUnclosed) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "[\"\\"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, NumberHugePosExponent) {
  Arena::Scope temp = Arena::getScratch();

  {
    JsonValue res;
    Slice<char> src = "1.0e9999999"_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(rc);
    CHECK(res.type == JsonType::Number);
    CHECK(isinf(res.number));
  }
  {
    JsonValue res;
    Slice<char> src = "-1.0e9999999"_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(rc);
    CHECK(res.type == JsonType::Number);
    CHECK(isinf(res.number));
  }
}

SN_TEST(JsonParser, NumberHugeNegExponent) {
  Arena::Scope temp = Arena::getScratch();

  {
    JsonValue res;
    Slice<char> src = "1.0e-9999999"_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(rc);
    CHECK(res.type == JsonType::Number);
    CHECK(res.number == 0.0f);
  }

  {
    JsonValue res;
    Slice<char> src = "-1.0e-9999999"_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(rc);
    CHECK(res.type == JsonType::Number);
    CHECK(res.number == -0.0f);
  }
}

SN_TEST(JsonParser, UnescapedCtrlChars) {
  Arena::Scope temp = Arena::getScratch();

  {
    JsonValue res;
    Slice<char> src = "\"\n\""_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(!rc);
  }
  {
    JsonValue res;
    Slice<char> src = "\"\t\""_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(!rc);
  }
  {
    JsonValue res;
    Slice<char> src = "\"\b\""_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(!rc);
  }
  {
    JsonValue res;
    Slice<char> src = "\"\x7f\""_slice;
    bool rc = tryParseValue(temp, src, res);
    CHECK(rc);
  }
}

SN_TEST(JsonParser, ObjectExtraComma) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = "{\"k\":0,}"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonUtils, GetKeyValue) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = 
      "{\"num\": -1.5, \"str\": \"string\", \"arr\": [1,2], \"obj\": {}, "
      "\"nul\": null, \"false\": false, \"tru\": true}"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);

  {
    JsonValue *v_num = getKeyValue(&res, "num"_slice);
    CHECK(v_num != nullptr);
    CHECK(isNumeric(v_num));
  }

  {
    JsonValue *v_str = getKeyValue(&res, "str"_slice);
    CHECK(v_str != nullptr);
    CHECK(isString(v_str));
  }

  {
    JsonValue *v_null = getKeyValue(&res, "nul"_slice);
    CHECK(v_null != nullptr);
    CHECK(isNull(v_null));
  }

  {
    JsonValue *v = getKeyValue(&res, "nonexistent"_slice);
    CHECK(v == nullptr);
  }
}

SN_TEST(JsonUtils, GetKeyValueOfType) {
  Arena::Scope temp = Arena::getScratch();

  JsonValue res;
  Slice<char> src = 
      "{\"num\": -1.5, \"str\": \"string\", \"arr\": [1,2], \"obj\": {}, "
      "\"nul\": null, \"false\": false, \"tru\": true}"_slice;
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);

  JsonValue *v_arr =
      getKeyValueOfType(&res, "arr"_slice, JsonType::Array);
  CHECK(v_arr != nullptr);
  CHECK(isArray(v_arr));
  CHECK(v_arr->array().length == 2);

  JsonValue *v_invalid =
      getKeyValueOfType(&res, "arr"_slice, JsonType::Object);
  CHECK(v_invalid == nullptr);
}
