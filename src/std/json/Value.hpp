#pragma once

#include "std/Slice.hpp"
#include "std/Types.h"

enum class JsonType : u8 {
  Null,
  False,
  True,
  Number,
  String,
  Array,
  Object,
};

struct JsonValue;
struct JsonKeyValue;

struct JsonValue {
  union {
    f64 number;
    // Points to utf-8 data of length `length`. Valid when type is String
    const char *str;
    // Points to JSON values of count `length`. Valid when type is Array
    JsonValue *arr;
    // Points to key-value pairs of count `length`. Valid when type is Object
    JsonKeyValue *kv;
  };
  // - When `type` is String, this is the byte length of the utf-8 string
  // - When `type` is Array, this is the number of values in the array
  // - When `type` is Object, this is the number of key-values on the object
  u32 length;
  JsonType type;

  Slice<const char> string() const { return {str, length}; }
  Slice<JsonValue> array() const { return {arr, length}; }
  Slice<JsonKeyValue> object() const { return {kv, length}; }
};

struct JsonKeyValue {
  Slice<const char> key;
  JsonValue value;
};
