#include "std/json/Parser.hpp"
#include "std/Arena.h"
#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"
#include "std/Types.h"
#include "std/Vector.hpp"
#include "std/VectorUtils.hpp"
#include "std/json/Value.hpp"

#include <cmath> // pow

static constexpr Slice<const char> NUL = sliceFromConstChar("null");
static constexpr Slice<const char> FALSE = sliceFromConstChar("false");
static constexpr Slice<const char> TRUE = sliceFromConstChar("true");

static void eatWhitespace(Slice<const char> &json) {
  while (json.length != 0) {
    switch (json[0]) {
      case ' ':
      case '\n':
      case '\r':
      case '\t':
        shrinkFromLeft(&json);
        break;
      default:
        return;
    }
  }
}

static bool isDigit(Slice<const char> json) {
  if (empty(json)) {
    return false;
  }

  return '0' <= json[0] && json[0] <= '9';
}

static bool isDigitExceptZero(Slice<const char> json) {
  if (empty(json)) {
    return false;
  }

  return '1' <= json[0] && json[0] <= '9';
}

static bool isHex(Slice<const char> json, u8 &out) {
  if (empty(json)) {
    return false;
  }

  if (isDigit(json)) {
    out = json[0] - '0';
    return true;
  }

  char c = json[0];

  if ('a' <= c && c <= 'f') {
    return 10 + (c - 'a');
  }

  if ('A' <= c && c <= 'F') {
    return 10 + (c - 'A');
  }

  return false;
}

/**
 * \brief When its lifetime ends, it rolls back the state of the referenced
 * Slice to a saved value (what it was when this Rollback object was
 * constructed), unless it was disarmed.
 */
struct Rollback {
  Slice<const char> *dst;
  const Slice<const char> saved;
  Rollback(Slice<const char> &subject) : dst(&subject), saved(subject) {}

  ~Rollback() {
    if (dst) {
      *dst = saved;
    }
  }

  void disarm() { dst = nullptr; }
};

static bool tryParseNumber(Slice<const char> &json, JsonValue &out) {
  out.type = JsonType::Number;

  if (json.empty()) {
    return false;
  }

  Rollback rollback(json);

  f64 sign = +1;

  if (json[0] == '-') {
    sign = -1;
    json.shrinkFromLeft();
  }

  if (json.empty()) {
    return false;
  }

  u64 whole = 0;
  f64 fraction = 0.0;

  if (json[0] == '0') {
    whole = 0;
    json.shrinkFromLeft();
  } else if (isDigitExceptZero(json)) {
    while (isDigit(json)) {
      whole *= 10;
      whole += (json[0] - '0');
      json.shrinkFromLeft();
    }

    if (empty(json)) {
      rollback.disarm();
      return true;
    }
  } else {
    return false;
  }

  if (empty(json)) {
    out.number = sign * f64(whole);
    rollback.disarm();
    return true;
  }

  if (json[0] == '.') {
    json.shrinkFromLeft();

    const f64 rcp10 = 1.0 / 10.0;
    f64 rcpDivisor = rcp10;
    while (isDigit(json)) {
      fraction += (json[0] - '0') * rcpDivisor;
      json.shrinkFromLeft();
      rcpDivisor *= rcp10;
    }
  }

  f64 combined = sign * (f64(whole) + fraction);
  out.number = combined;

  if (empty(json)) {
    rollback.disarm();
    return true;
  }

  if (json[0] == 'e' || json[0] == 'E') {
    json.shrinkFromLeft();

    if (json.empty()) {
      return false;
    }

    i32 exponentSign = 1;
    i32 exponent = 0;

    if (json[0] == '+') {
      json.shrinkFromLeft();
    } else if (json[0] == '-') {
      exponentSign = -1;
      json.shrinkFromLeft();
    }

    if (empty(json) || !isDigit(json)) {
      return false;
    }

    while (isDigit(json)) {
      exponent *= 10;
      exponent += (json[0] - '0');
      json.shrinkFromLeft();
    }

    out.number = combined * pow(10, exponentSign * exponent);
  }

  rollback.disarm();
  return true;
}

static bool tryParseUnicodeCodepoint(Slice<const char> &json, u32 &out) {
  if (json.length < 5 || json[0] != 'u') {
    return false;
  }

  Rollback rollback(json);

  json.shrinkFromLeft();  // eat u

  out = 0;
  for (u32 i = 0; i < 4; i++) {
    u8 digit;
    if (!isHex(json, digit)) {
      return false;
    }

    DCHECK(digit < 16);
    out = (out << 4) | digit;
    json.shrinkFromLeft();
  }

  rollback.disarm();
  return true;
}

static bool tryParseEscapedChar(Slice<const char> &json, u32 &out) {
  if (json.empty() || json[0] != '\\') {
    return false;
  }

  Slice<const char> saved = json;
  json.shrinkFromLeft();

  switch (json[0]) {
    case '"':
    case '\\':
    case '/':
      out = u8(json[0]);
      json.shrinkFromLeft();
      return true;
    case 'b':
      out = '\b';
      json.shrinkFromLeft();
      return true;
    case 'f':
      out = '\f';
      json.shrinkFromLeft();
      return true;
    case 'n':
      out = '\n';
      json.shrinkFromLeft();
      return true;
    case 'r':
      out = '\r';
      json.shrinkFromLeft();
      return true;
    case 't':
      out = '\t';
      json.shrinkFromLeft();
      return true;
    case 'u':
      return tryParseUnicodeCodepoint(json, out);
    default:
      break;
  }

  json = saved;
  return false;
}

static void encodeToUtf8(Arena *arena, Vector<char> *vec, u32 codepoint) {
  auto pushByte = [arena, vec](u8 b) {
    char tmp;
    memcpy(&tmp, &b, 1);
    appendVal(arena, vec, tmp);
  };

  if (codepoint <= 0x7F) {
    // 1 byte: 0xxxxxxx
    pushByte(u8(codepoint));
  } else if (codepoint <= 0x7FF) {
    // 2 bytes: 110xxxxx 10xxxxxx
    pushByte(0xC0 | (codepoint >> 6));
    pushByte(0x80 | (codepoint & 0x3F));
  } else if (codepoint <= 0xFFFF) {
    // 3 bytes: 1110xxxx 10xxxxxx 10xxxxxx
    pushByte(0xE0 | (codepoint >> 12));
    pushByte(0x80 | ((codepoint >> 6) & 0x3F));
    pushByte(0x80 | (codepoint & 0x3F));
  } else if (codepoint <= 0x10FFFF) {
    // 4 bytes: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    pushByte(0xF0 | (codepoint >> 18));
    pushByte(0x80 | ((codepoint >> 12) & 0x3F));
    pushByte(0x80 | ((codepoint >> 6) & 0x3F));
    pushByte(0x80 | (codepoint & 0x3F));
  }
}

static bool tryParseString(Arena *arena,
                           Arena *tempArena,
                           Slice<const char> &json,
                           JsonValue &out) {
  Arena::Scope temp = tempArena;

  if (empty(json) || json[0] != '"') {
    return false;
  }

  Rollback rollback(json);

  out.type = JsonType::String;
  out.length = 0;
  out.str = nullptr;
  json.shrinkFromLeft();  // eat opening quotes

  if (json.empty()) {
    return false;
  }

  if (json[0] == '"') {
    json.shrinkFromLeft();  // eat closing quotes
    rollback.disarm();
    return true;
  }

  Vector<char> vec;

  while (!json.empty() && json[0] != '"') {
    switch (json[0]) {
      case '\\': {
        u32 codepoint;
        if (!tryParseEscapedChar(json, codepoint)) {
          return false;
        }
        encodeToUtf8(temp, &vec, codepoint);
        break;
      }
      default: {
        appendVal(temp, &vec, json[0]);
        json.shrinkFromLeft();
        break;
      }
    }
  }

  if (json.empty()) {
    return false;
  }

  json.shrinkFromLeft();  // eat closing quotes

  Slice<const char> contents = copyToSlice(arena, vec).asConst();
  out.length = contents.length;
  out.str = contents.data;

  rollback.disarm();
  return true;
}

bool tryParseValue(Arena *arena,
                   Arena *tempArena,
                   Slice<const char> &json,
                   JsonValue &out);

static bool tryParseArray(Arena *arena,
                          Arena *tempArena,
                          Slice<const char> &json,
                          JsonValue &out) {
  if (empty(json) || json[0] != '[') {
    return false;
  }

  Rollback rollback(json);
  json.shrinkFromLeft();  // eat [

  out.type = JsonType::Array;
  out.length = 0;
  out.arr = nullptr;

  eatWhitespace(json);
  if (json.empty()) {
    return false;
  }

  if (json[0] == ']') {
    json.shrinkFromLeft();  // eat ]
    return true;
  }

  Arena::Scope temp = tempArena;
  Vector<JsonValue> values;

  while (!json.empty() && json[0] != ']') {
    JsonValue *pValue = append(temp, &values);
    if (!tryParseValue(arena, tempArena, json, *pValue)) {
      return false;
    }

    if (json.empty()) {
      return false;
    }

    eatWhitespace(json);

    if (json[0] == ']') {
      break;
    }

    if (json[0] != ',') {
      return false;
    }

    json.shrinkFromLeft();  // eat comma
  }

  json.shrinkFromLeft();  // eat ]

  Slice<JsonValue> contents = copyToSlice(arena, values);
  out.length = contents.length;
  out.arr = contents.data;

  rollback.disarm();
  return true;
}

static bool tryParseObject(Arena *arena,
                           Arena *tempArena,
                           Slice<const char> &json,
                           JsonValue &out) {
  if (json.empty() || json[0] != '{') {
    return false;
  }

  Rollback rollback(json);

  json.shrinkFromLeft();
  out.type = JsonType::Object;
  out.length = 0;
  out.kv = nullptr;

  eatWhitespace(json);

  if (empty(json)) {
    return false;
  }

  if (json[0] == '}') {
    shrinkFromLeft(&json);
    rollback.disarm();
    return true;
  }

  Arena::Scope temp = tempArena;
  Vector<JsonKeyValue> entries;

  while (!json.empty() && json[0] != '}') {
    eatWhitespace(json);

    JsonKeyValue *kv = append(temp, &entries);
    JsonValue key = {};

    if (!tryParseString(arena, tempArena, json, key)) {
      return false;
    }

    DCHECK(key.type == JsonType::String);
    kv->key = key.string();

    eatWhitespace(json);

    if (empty(json) || json[0] != ':') {
      return false;
    }

    json.shrinkFromLeft();
    eatWhitespace(json);

    if (!tryParseValue(arena, tempArena, json, kv->value)) {
      return false;
    }

    if (json.empty()) {
      return false;
    }

    eatWhitespace(json);

    if (json[0] == '}') {
      break;
    }

    if (json[0] != ',') {
      return false;
    }

    json.shrinkFromLeft();  // eat comma
  }

  if (json.empty()) {
    return false;
  }

  json.shrinkFromLeft();  // eat }

  Slice<JsonKeyValue> contents = copyToSlice(arena, entries);
  out.length = contents.length;
  out.kv = contents.data;

  rollback.disarm();
  return true;
}

bool tryParseValue(Arena *arena,
                   Arena *tempArena,
                   Slice<const char> &json,
                   JsonValue &out) {
  if (json.empty()) {
    return false;
  }

  Slice<const char> saved = json;
  eatWhitespace(json);

  if (tryParseNumber(json, out)) {
    return true;
  }

  if (compareAsString(subarray(json, 0, 4), NUL)) {
    out.type = JsonType::Null;

    shrinkFromLeftByCount(&json, NUL.length);
    eatWhitespace(json);
    return true;
  }

  if (compareAsString(subarray(json, 0, 5), FALSE)) {
    out.type = JsonType::False;

    shrinkFromLeftByCount(&json, FALSE.length);
    eatWhitespace(json);
    return true;
  }

  if (compareAsString(subarray(json, 0, 4), TRUE)) {
    out.type = JsonType::True;

    shrinkFromLeftByCount(&json, TRUE.length);
    eatWhitespace(json);
    return true;
  }

  if (tryParseString(arena, tempArena, json, out)) {
    eatWhitespace(json);
    return true;
  }

  if (tryParseObject(arena, tempArena, json, out)) {
    eatWhitespace(json);
    return true;
  }

  if (tryParseArray(arena, tempArena, json, out)) {
    eatWhitespace(json);
    return true;
  }

  json = saved;
  return false;
}

bool tryParseValue(Arena *arena, Slice<const char> &json, JsonValue &out) {
  Arena::Scope temp = getScratch(&arena, 1);
  return tryParseValue(arena, temp, json, out);
}
