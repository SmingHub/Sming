// Copyright Benoit Blanchon 2014-2016
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson
// If you like this project, please add a star!

#pragma once

#include "Arduino/String.hpp"

namespace ArduinoJson {

// Represents a key in a JsonObject
class JsonObjectKey {
 public:
  JsonObjectKey(const char* key) : _value(key), _needs_copy(false) {}
  JsonObjectKey(const String& key) : _value(key.c_str()), _needs_copy(true) {}

  /** @brief This constructor allows a FlashString to be used directly as a key parameter without any further casting.
   *
   * @note Would like to use stack for this but buffer doesn't stay around long enough when used
   * with JsonObject::createNestedArray()
   *
   * According to the documentation, the key parameter must be either a const char* or a const String&.
   *
   * Note also that despite the documentation, ArduinoJson does _not_ create a copy when assiging char* values:
   *
   *   DEFINE_STRING_P(test, "This is a test value")
   *   json["test"] = _FS(test);	// Doesn't work, we end up with garbage
   *   json["test"] = String(test);	// OK
   *
   * Hopefully this will all be resolved in version 6.
   *
   * 	https://arduinojson.org/v5/api/jsonobject/subscript/
   * 	https://arduinojson.org/v5/api/jsonobject/createnestedarray/
   * 	https://arduinojson.org/v5/api/jsonobject/createnestedobject/
   *
   */
  JsonObjectKey(const FlashString& key) : _value(String(key).c_str()), _needs_copy(true) {}

  const char* c_str() const { return _value; }
  bool needs_copy() const { return _needs_copy; }

 private:
  const char* _value;
  bool _needs_copy;
};
}
