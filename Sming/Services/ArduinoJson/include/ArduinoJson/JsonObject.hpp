// Copyright Benoit Blanchon 2014-2015
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson

#pragma once

#include "Internals/JsonBufferAllocated.hpp"
#include "Internals/JsonPrintable.hpp"
#include "Internals/List.hpp"
#include "Internals/ReferenceType.hpp"
#include "JsonPair.hpp"

// Returns the size (in bytes) of an object with n elements.
// Can be very handy to determine the size of a StaticJsonBuffer.
#define JSON_OBJECT_SIZE(NUMBER_OF_ELEMENTS) \
  (sizeof(JsonObject) + (NUMBER_OF_ELEMENTS) * sizeof(JsonObject::node_type))

namespace ArduinoJson {

// Forward declarations
class JsonArray;
class JsonBuffer;

// A dictionary of JsonVariant indexed by string (char*)
//
// The constructor is private, instances must be created via
// JsonBuffer::createObject() or JsonBuffer::parseObject().
// A JsonObject can be serialized to a JSON string via JsonObject::printTo().
// It can also be deserialized from a JSON string via JsonBuffer::parseObject().
class JsonObject : public Internals::JsonPrintable<JsonObject>,
                   public Internals::ReferenceType,
                   public Internals::List<JsonPair>,
                   public Internals::JsonBufferAllocated {
  // JsonBuffer is a friend because it needs to call the private constructor.
  friend class JsonBuffer;

 public:
  typedef const char *key_type;
  typedef JsonPair value_type;

  // Gets the JsonVariant associated with the specified key.
  // Returns a reference or JsonVariant::invalid() if not found.
  JsonVariant &at(key_type key);

  // Gets the JsonVariant associated with the specified key.
  // Returns a constant reference or JsonVariant::invalid() if not found.
  const JsonVariant &at(key_type key) const;

  // Gets or create the JsonVariant associated with the specified key.
  // Returns a reference or JsonVariant::invalid() if allocation failed.
  JsonVariant &operator[](key_type key);
  JsonVariant &operator[](const String& key);

  // Gets the JsonVariant associated with the specified key.
  // Returns a constant reference or JsonVariant::invalid() if not found.
  const JsonVariant &operator[](key_type key) const { return at(key); }

  // Adds an uninitialized JsonVariant associated with the specified key.
  // Return a reference or JsonVariant::invalid() if allocation fails.
  JsonVariant &add(key_type key) { return (*this)[key]; }
  JsonVariant &add(const String& key);

  // Adds the specified key with the specified value.
  template <typename T>
  void add(key_type key, T value) {
    add(key).set(value);
  }
  template <typename T>
  void add(const String& key, T value) {
    add(key).set(value);
  }

  // Adds the specified key with a reference to the specified JsonArray.
  void add(key_type key, JsonArray &array) { add(key).set(array); }
  void add(const String& stringKey, JsonArray &array) { add(stringKey).set(array); }

  // Adds the specified key with a reference to the specified JsonObject.
  void add(key_type key, JsonObject &object) { add(key).set(object); }
  void add(const String& stringKey, JsonObject &object) { add(stringKey).set(object); }

  // Add item by value: create copy of string (in JSON buffer memory) and add it to object childs
  // memory will be freed automatically
  void addCopy(key_type stringKey, const String &stringVal);
  void addCopy(const String& stringKey, const String &stringVal);

  // Creates and adds a JsonArray.
  // This is a shortcut for JsonBuffer::createArray() and JsonObject::add().
  JsonArray &createNestedArray(key_type key);
  JsonArray &createNestedArray(const String& stringKey);

  // Creates and adds a JsonObject.
  // This is a shortcut for JsonBuffer::createObject() and JsonObject::add().
  JsonObject &createNestedObject(key_type key);
  JsonObject &createNestedObject(const String& stringKey);

  // Tells weither the specified key is present and associated with a value.
  bool containsKey(key_type key) const { return at(key).success(); }

  // Removes the specified key and the associated value.
  void remove(key_type key);

  // Returns a reference an invalid JsonObject.
  // This object is meant to replace a NULL pointer.
  // This is used when memory allocation or JSON parsing fail.
  static JsonObject &invalid() { return _invalid; }

  // Serialize the object to the specified JsonWriter
  void writeTo(Internals::JsonWriter &writer) const;

  String toJsonString(bool prettyPrintStyle = true) const;

 protected:
  // Don't use this methods. Convert value to "const char*" (if it will be available) or use "addCopy" instead
  void add(key_type key, const String &stringVal) { add(key).set(stringVal); }
  void add(const String& stringKey, const String &stringVal) { add(stringKey).set(stringVal); }


 private:
  // Create an empty JsonArray attached to the specified JsonBuffer.
  explicit JsonObject(JsonBuffer *buffer) : Internals::List<JsonPair>(buffer) {}

  // Returns the list node that matches the specified key.
  node_type *getNodeAt(key_type key) const;

  // The instance returned by JsonObject::invalid()
  static JsonObject _invalid;
};
}
