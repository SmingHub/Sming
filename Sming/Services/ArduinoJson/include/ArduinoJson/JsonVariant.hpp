// Copyright Benoit Blanchon 2014-2016
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson
// If you like this project, please add a star!

#pragma once

#include <stddef.h>
#include <stdint.h>  // for uint8_t

#include "Internals/JsonPrintable.hpp"
#include "Internals/JsonVariantContent.hpp"
#include "Internals/JsonVariantType.hpp"
#include "Internals/Unparsed.hpp"
#include "JsonVariantBase.hpp"
#include "TypeTraits/EnableIf.hpp"
#include "TypeTraits/IsFloatingPoint.hpp"
#include "TypeTraits/IsIntegral.hpp"
#include "TypeTraits/IsSame.hpp"
#include "TypeTraits/RemoveConst.hpp"
#include "TypeTraits/RemoveReference.hpp"

namespace ArduinoJson {

// Forward declarations.
class JsonArray;
class JsonObject;

// A variant that can be a any value serializable to a JSON value.
//
// It can be set to:
// - a boolean
// - a char, short, int or a long (signed or unsigned)
// - a string (const char*)
// - a reference to a JsonArray or JsonObject
class JsonVariant : public JsonVariantBase<JsonVariant> {
 public:
  template <typename T>
  struct IsConstructibleFrom;

  // Creates an uninitialized JsonVariant
  FORCE_INLINE JsonVariant() : _type(Internals::JSON_UNDEFINED) {}

  // Create a JsonVariant containing a boolean value.
  // It will be serialized as "true" or "false" in JSON.
  FORCE_INLINE JsonVariant(bool value);

  // Create a JsonVariant containing a floating point value.
  // The second argument specifies the number of decimal digits to write in
  // the JSON string.
  // JsonVariant(double value, uint8_t decimals);
  // JsonVariant(float value, uint8_t decimals);
  template <typename T>
  FORCE_INLINE JsonVariant(
      T value, uint8_t decimals = 2,
      typename TypeTraits::EnableIf<TypeTraits::IsFloatingPoint<T>::value>::type
          * = 0) {
    using namespace Internals;
    _type = static_cast<JsonVariantType>(JSON_FLOAT_0_DECIMALS + decimals);
    _content.asFloat = static_cast<JsonFloat>(value);
  }

  // Create a JsonVariant containing an integer value.
  // JsonVariant(short)
  // JsonVariant(int)
  // JsonVariant(long)
  template <typename T>
  FORCE_INLINE JsonVariant(
      T value,
      typename TypeTraits::EnableIf<TypeTraits::IsIntegral<T>::value>::type * =
          0) {
    using namespace Internals;
    _type = JSON_INTEGER;
    _content.asInteger = static_cast<JsonInteger>(value);
  }

  // Create a JsonVariant containing a string.
  FORCE_INLINE JsonVariant(const char *value);

  // Create a JsonVariant containing an unparsed string
  FORCE_INLINE JsonVariant(Internals::Unparsed value);

  // Create a JsonVariant containing a reference to an array.
  FORCE_INLINE JsonVariant(JsonArray &array);

  // Create a JsonVariant containing a reference to an object.
  FORCE_INLINE JsonVariant(JsonObject &object);

  // Get the variant as the specified type.
  // short as<short>() const;
  // int as<int>() const;
  // long as<long>() const;
  template <typename T>
  const typename TypeTraits::EnableIf<TypeTraits::IsIntegral<T>::value, T>::type
  as() const {
    return static_cast<T>(asInteger());
  }
  // double as<double>() const;
  // float as<float>() const;
  template <typename T>
  const typename TypeTraits::EnableIf<TypeTraits::IsFloatingPoint<T>::value,
                                      T>::type
  as() const {
    return static_cast<T>(asFloat());
  }
  // const String as<String>() const;
  template <typename T>
  const typename TypeTraits::EnableIf<TypeTraits::IsSame<T, String>::value,
                                      T>::type
  as() const {
    return toString();
  }
  // const char* as<const char*>() const;
  // const char* as<char*>() const;
  template <typename T>
  typename TypeTraits::EnableIf<TypeTraits::IsSame<T, const char *>::value,
                                const char *>::type
  as() const {
    return asString();
  }
  // const bool as<bool>() const
  template <typename T>
  const typename TypeTraits::EnableIf<TypeTraits::IsSame<T, bool>::value,
                                      T>::type
  as() const {
    return asInteger() != 0;
  }
  // JsonArray& as<JsonArray> const;
  // JsonArray& as<JsonArray&> const;
  // JsonArray& as<const JsonArray&> const;
  template <typename T>
  typename TypeTraits::EnableIf<
      TypeTraits::IsSame<
          typename TypeTraits::RemoveConst<
              typename TypeTraits::RemoveReference<T>::type>::type,
          JsonArray>::value,
      JsonArray &>::type
  as() const {
    return asArray();
  }
  // JsonObject& as<JsonObject> const;
  // JsonObject& as<JsonObject&> const;
  // JsonObject& as<const JsonObject&> const;
  template <typename T>
  typename TypeTraits::EnableIf<
      TypeTraits::IsSame<
          typename TypeTraits::RemoveConst<
              typename TypeTraits::RemoveReference<T>::type>::type,
          JsonObject>::value,
      JsonObject &>::type
  as() const {
    return asObject();
  }

  // Tells weither the variant has the specified type.
  // Returns true if the variant has type type T, false otherwise.
  template <typename T>
  bool is() const;

  // Serialize the variant to a JsonWriter
  void writeTo(Internals::JsonWriter &writer) const;

  // TODO: rename
  template <typename T>
  static T invalid();

  const char *asString() const;
  JsonArray &asArray() const;
  JsonObject &asObject() const;

 private:
  String toString() const;
  Internals::JsonFloat asFloat() const;
  Internals::JsonInteger asInteger() const;

  // The current type of the variant
  Internals::JsonVariantType _type;

  // The various alternatives for the value of the variant.
  Internals::JsonVariantContent _content;
};

inline JsonVariant float_with_n_digits(float value, uint8_t digits) {
  return JsonVariant(value, digits);
}

inline JsonVariant double_with_n_digits(double value, uint8_t digits) {
  return JsonVariant(value, digits);
}

template <typename T>
struct JsonVariant::IsConstructibleFrom {
  static const bool value =
      TypeTraits::IsIntegral<T>::value ||
      TypeTraits::IsFloatingPoint<T>::value ||
      TypeTraits::IsSame<T, bool>::value ||
      TypeTraits::IsSame<T, char *>::value ||
      TypeTraits::IsSame<T, const char *>::value ||
      TypeTraits::IsSame<T, JsonArray &>::value ||
      TypeTraits::IsSame<T, const JsonArray &>::value ||
      TypeTraits::IsSame<T, JsonArraySubscript &>::value ||
      TypeTraits::IsSame<T, const JsonArraySubscript &>::value ||
      TypeTraits::IsSame<T, JsonObject &>::value ||
      TypeTraits::IsSame<T, const JsonObject &>::value ||
      TypeTraits::IsSame<T, JsonObjectSubscript<const char *> &>::value ||
      TypeTraits::IsSame<T, const JsonObjectSubscript<const char *> &>::value ||
      TypeTraits::IsSame<T, JsonObjectSubscript<String> &>::value ||
      TypeTraits::IsSame<T, const JsonObjectSubscript<String> &>::value ||
      TypeTraits::IsSame<T, JsonVariant &>::value ||
      TypeTraits::IsSame<T, const JsonVariant &>::value;
};
}

// Include inline implementations
#include "JsonVariant.ipp"
