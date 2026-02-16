#pragma once

#include "std/SliceUtils.hpp"
#include "std/json/Value.hpp"

inline bool hasType(const JsonValue *v, JsonType t) {
  return v->type == t;
}

inline bool isNull(const JsonValue *v) {
  return hasType(v, JsonType::Null);
}

inline bool isBoolean(const JsonValue *v) {
  return hasType(v, JsonType::False) || hasType(v, JsonType::True);
}

inline bool isNumeric(const JsonValue *v) {
  return hasType(v, JsonType::Number);
}

inline bool isString(const JsonValue *v) {
  return hasType(v, JsonType::String);
}

inline bool isArray(const JsonValue *v) {
  return hasType(v, JsonType::Array);
}

inline bool isObject(const JsonValue *v) {
  return hasType(v, JsonType::Object);
}

inline bool isNull(const JsonValue &v) {
  return isNull(&v);
}

inline bool isBoolean(const JsonValue &v) {
  return isBoolean(&v);
}

inline bool isNumeric(const JsonValue &v) {
  return isNumeric(&v);
}

inline bool isString(const JsonValue &v) {
  return isString(&v);
}

inline bool isArray(const JsonValue &v) {
  return isArray(&v);
}

inline bool isObject(const JsonValue &v) {
  return isObject(&v);
}

template <typename T>
inline T asNumeric(const JsonValue *v) {
  return T(v->number);
}

template <typename T>
inline T asNumeric(const JsonValue &v) {
  return asNumeric<T>(&v);
}

inline f32 coerceToF32(const JsonValue *v) {
  switch (v->type) {
    case JsonType::Number:
      return f32(v->number);
    case JsonType::True:
      return 1.0f;
    default:
      return 0.0f;
  }
}

inline f32 coerceToF32(const JsonValue &v) {
  return coerceToF32(&v);
}

inline u32 length(const JsonValue *v) {
  return v->length;
}

inline bool getKeyValue(JsonValue *obj,
                        Slice<const char> key,
                        JsonValue **out) {
  *out = nullptr;

  if (obj->type != JsonType::Object) {
    return false;
  }

  for (auto [kv, _] : obj->object()) {
    if (compareAsString(kv.key, key)) {
      *out = &kv.value;
      return true;
    }
  }

  return false;
}

inline JsonValue *getKeyValue(JsonValue *obj, Slice<const char> key) {
  if (obj->type != JsonType::Object) {
    return nullptr;
  }

  for (auto [kv, _] : obj->object()) {
    if (compareAsString(kv.key, key)) {
      return &kv.value;
    }
  }

  return nullptr;
}

inline JsonValue *getKeyValueOfType(JsonValue *obj,
                                    Slice<const char> key,
                                    JsonType Type) {
  JsonValue *ret = getKeyValue(obj, key);
  if (ret == nullptr) {
    return nullptr;
  }

  if (!hasType(ret, Type)) {
    return nullptr;
  }

  return ret;
}
