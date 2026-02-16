#include "std/Testing.hpp"

#include "std/json/Value.hpp"
#include "std/json/Parser.hpp"
#include "std/json/Utils.hpp"

#define CHECK_STR(Var, Literal) \
  CHECK((Var).asConst() == Slice<const char>(sliceFromConstChar(Literal)))

SN_TEST(JsonParser, EmptyObject) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("{}");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Object);
  CHECK(res.object().empty());
}

SN_TEST(JsonParser, EmptyArray) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[]");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  CHECK(res.array().empty());
}

SN_TEST(JsonParser, OpenArray) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[");
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, OpenArrayNested) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[[[[[[[[]]");
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, OpenObject) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("{");
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, OpenObjectNested) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src =
      sliceFromConstChar("{\"k\": {\"k\": {\"k\": {\"k\": {");
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, NumberArray) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src =
      sliceFromConstChar("[1, 2.0, 1e+8, -4, -5.0, -6.7e+2]");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  Slice<JsonValue> elems = res.array();
  CHECK(elems.length == 6);

  for (auto [e, _] : elems) {
    CHECK(isNumeric(e));
  }

  CHECK(elems[0].number == 1);
  CHECK(elems[1].number == 2);
  CHECK(elems[2].number == 1e+8);
  CHECK(elems[3].number == -4);
  CHECK(elems[4].number == -5.0);
  CHECK(elems[5].number == -6.7e+2);
}

SN_TEST(JsonParser, EmptyArrayNewLine) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[\n]");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  Slice<JsonValue> elems = res.array();
  CHECK(elems.empty());
}

SN_TEST(JsonParser, EmptyArrayWhitespace) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src =
      sliceFromConstChar("     \n      \n \n\n \r\n  [    \n ]    \n\n");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Array);
  Slice<JsonValue> elems = res.array();
  CHECK(elems.empty());
}

SN_TEST(JsonParser, StringEmpty) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("\"\"");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::String);
  CHECK(res.string().empty());
}

SN_TEST(JsonParser, StringSimple) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("\"asd\"");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::String);
  CHECK(res.string() == Slice<const char>(sliceFromConstChar("asd")));
}

SN_TEST(JsonParser, StringCodepoint) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("\"\\u0391\"");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::String);

  const u8 uc[2] = {0xCE, 0x91};
  Slice<const u8> ucs = sliceFrom(uc);
  CHECK(res.string() == ucs.cast<const char>());
}

SN_TEST(JsonParser, StringArray) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[\"asd\", \"\", \"\\u0391\"]");
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
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar(
      "{\"num\": -1.5, \"str\": \"string\", \"arr\": [1,2], \"obj\": {}, "
      "\"nul\": null, \"false\": false, \"tru\": true}");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(res.type == JsonType::Object);
  Slice<JsonKeyValue> kvs = res.object();
  CHECK(kvs.length == 7);

  CHECK_STR(kvs[0].key, "num");
  CHECK(kvs[0].value.type == JsonType::Number);
  CHECK(kvs[0].value.number == -1.5);

  CHECK_STR(kvs[1].key, "str");
  CHECK(kvs[1].value.type == JsonType::String);
  CHECK_STR(kvs[1].value.string(), "string");

  CHECK_STR(kvs[2].key, "arr");
  CHECK(kvs[2].value.type == JsonType::Array);
  CHECK(kvs[2].value.array().length == 2);
  CHECK(kvs[2].value.array()[0].type == JsonType::Number);
  CHECK(kvs[2].value.array()[0].number == 1);
  CHECK(kvs[2].value.array()[1].type == JsonType::Number);
  CHECK(kvs[2].value.array()[1].number == 2);

  CHECK_STR(kvs[3].key, "obj");
  CHECK(kvs[3].value.type == JsonType::Object);
  CHECK(kvs[3].value.object().empty());

  CHECK_STR(kvs[4].key, "nul");
  CHECK(kvs[4].value.type == JsonType::Null);

  CHECK_STR(kvs[5].key, "false");
  CHECK(kvs[5].value.type == JsonType::False);

  CHECK_STR(kvs[6].key, "tru");
  CHECK(kvs[6].value.type == JsonType::True);
}

SN_TEST(JsonParser, StringUtf8) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char8_t> src8 = sliceFromConstChar(u8"[\"€𝄞\"]");
  Slice<const char> src = src8.cast<const char>();
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);

  CHECK(res.type == JsonType::Array);
  CHECK(res.array().length == 1);
  Slice<const char8_t> expected = sliceFromConstChar(u8"€𝄞");
  CHECK(res.array()[0].string() == expected.cast<const char>());
}

SN_TEST(JsonParser, ArrayDoubleComma) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[1,,2]");
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, ArrayDoubleClose) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[\"x\"]]");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);
  CHECK(src.length == 1);
}

SN_TEST(JsonParser, ArrayIncomplete) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[\"x\"");
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonParser, NumberWithAlpha) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar("[1.2a-3]");
  bool rc = tryParseValue(temp, src, res);
  CHECK(!rc);
}

SN_TEST(JsonUtils, GetKeyValue) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar(
      "{\"num\": -1.5, \"str\": \"string\", \"arr\": [1,2], \"obj\": {}, "
      "\"nul\": null, \"false\": false, \"tru\": true}");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);

  {
    JsonValue *v_num = getKeyValue(&res, sliceFromConstChar("num"));
    CHECK(v_num != nullptr);
    CHECK(isNumeric(v_num));
  }

  {
    JsonValue *v_str = getKeyValue(&res, sliceFromConstChar("str"));
    CHECK(v_str != nullptr);
    CHECK(isString(v_str));
  }

  {
    JsonValue *v_null = getKeyValue(&res, sliceFromConstChar("nul"));
    CHECK(v_null != nullptr);
    CHECK(isNull(v_null));
  }

  {
    JsonValue *v = getKeyValue(&res, sliceFromConstChar("nonexistent"));
    CHECK(v == nullptr);
  }
}

SN_TEST(JsonUtils, GetKeyValueOfType) {
  Arena::Scope temp = getScratch(nullptr, 0);

  JsonValue res;
  Slice<const char> src = sliceFromConstChar(
      "{\"num\": -1.5, \"str\": \"string\", \"arr\": [1,2], \"obj\": {}, "
      "\"nul\": null, \"false\": false, \"tru\": true}");
  bool rc = tryParseValue(temp, src, res);
  CHECK(rc);

  JsonValue *v_arr =
      getKeyValueOfType(&res, sliceFromConstChar("arr"), JsonType::Array);
  CHECK(v_arr != nullptr);
  CHECK(isArray(v_arr));
  CHECK(v_arr->array().length == 2);

  JsonValue *v_invalid =
      getKeyValueOfType(&res, sliceFromConstChar("arr"), JsonType::Object);
  CHECK(v_invalid == nullptr);
}