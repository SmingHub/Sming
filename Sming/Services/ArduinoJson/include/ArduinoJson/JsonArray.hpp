// Copyright Benoit Blanchon 2014-2016
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson
// If you like this project, please add a star!

#pragma once

#include "Internals/JsonBufferAllocated.hpp"
#include "Internals/JsonPrintable.hpp"
#include "Internals/List.hpp"
#include "Internals/ReferenceType.hpp"
#include "JsonVariant.hpp"
#include "TypeTraits/EnableIf.hpp"
#include "TypeTraits/IsFloatingPoint.hpp"
#include "TypeTraits/IsReference.hpp"
#include "TypeTraits/IsSame.hpp"

// Returns the size (in bytes) of an array with n elements.
// Can be very handy to determine the size of a StaticJsonBuffer.
#define JSON_ARRAY_SIZE(NUMBER_OF_ELEMENTS) \
  (sizeof(JsonArray) + (NUMBER_OF_ELEMENTS) * sizeof(JsonArray::node_type))

namespace ArduinoJson {

// Forward declarations
class JsonObject;
class JsonBuffer;
class JsonArraySubscript;

// An array of JsonVariant.
//
// The constructor is private, instances must be created via
// JsonBuffer::createArray() or JsonBuffer::parseArray().
// A JsonArray can be serialized to a JSON string via JsonArray::printTo().
// It can also be deserialized from a JSON string via JsonBuffer::parseArray().
class JsonArray : public Internals::JsonPrintable<JsonArray>,
                  public Internals::ReferenceType,
                  public Internals::List<JsonVariant>,
                  public Internals::JsonBufferAllocated {
 public:
  // A meta-function that returns true if type T can be used in
  // JsonArray::set()
  template <typename T>
  struct CanSet {
    static const bool value = JsonVariant::IsConstructibleFrom<T>::value ||
                              TypeTraits::IsSame<T, String &>::value ||
                              TypeTraits::IsSame<T, const String &>::value;
  };

  // Create an empty JsonArray attached to the specified JsonBuffer.
  // You should not call this constructor directly.
  // Instead, use JsonBuffer::createArray() or JsonBuffer::parseArray().
  explicit JsonArray(JsonBuffer *buffer)
      : Internals::List<JsonVariant>(buffer) {}

  // Gets the value at the specified index
  FORCE_INLINE JsonVariant operator[](size_t index) const;

  // Gets or sets the value at specified index
  FORCE_INLINE JsonArraySubscript operator[](size_t index);

  // Adds the specified value at the end of the array.
  //
  // bool add(bool);
  // bool add(char);
  // bool add(long);
  // bool add(int);
  // bool add(short);
  // bool add(float value);
  // bool add(double value);
  // bool add(const char*);
  template <typename T>
  FORCE_INLINE bool add(
      T value,
      typename TypeTraits::EnableIf<
          CanSet<T>::value && !TypeTraits::IsReference<T>::value>::type * = 0) {
    return addNode<T>(value);
  }
  // bool add(const String&)
  // bool add(const JsonVariant&);
  // bool add(JsonArray&);
  // bool add(JsonObject&);
  template <typename T>
  FORCE_INLINE bool add(
      const T &value,
      typename TypeTraits::EnableIf<CanSet<T &>::value>::type * = 0) {
    return addNode<T &>(const_cast<T &>(value));
  }
  // bool add(float value, uint8_t decimals);
  // bool add(double value, uint8_t decimals);
  template <typename T>
  FORCE_INLINE bool add(
      T value, uint8_t decimals,
      typename TypeTraits::EnableIf<TypeTraits::IsFloatingPoint<T>::value>::type
          * = 0) {
    return addNode<JsonVariant>(JsonVariant(value, decimals));
  }

  // Sets the value at specified index.
  //
  // bool set(size_t index, bool value);
  // bool set(size_t index, long value);
  // bool set(size_t index, int value);
  // bool set(size_t index, short value);
  template <typename T>
  FORCE_INLINE bool set(
      size_t index, T value,
      typename TypeTraits::EnableIf<
          CanSet<T>::value && !TypeTraits::IsReference<T>::value>::type * = 0) {
    return setNodeAt<T>(index, value);
  }
  // bool set(size_t index, const String&)
  // bool set(size_t index, const JsonVariant&);
  // bool set(size_t index, JsonArray&);
  // bool set(size_t index, JsonObject&);
  template <typename T>
  FORCE_INLINE bool set(
      size_t index, const T &value,
      typename TypeTraits::EnableIf<CanSet<T &>::value>::type * = 0) {
    return setNodeAt<T &>(index, const_cast<T &>(value));
  }
  // bool set(size_t index, float value, uint8_t decimals = 2);
  // bool set(size_t index, double value, uint8_t decimals = 2);
  template <typename T>
  FORCE_INLINE bool set(
      size_t index, T value, uint8_t decimals,
      typename TypeTraits::EnableIf<TypeTraits::IsFloatingPoint<T>::value>::type
          * = 0) {
    return setNodeAt<const JsonVariant &>(index, JsonVariant(value, decimals));
  }

  // Gets the value at the specified index.
  FORCE_INLINE JsonVariant get(size_t index) const;

  // Gets the value at the specified index.
  template <typename T>
  FORCE_INLINE T get(size_t index) const;

  // Check the type of the value at specified index.
  template <typename T>
  FORCE_INLINE bool is(size_t index) const;

  // Creates a JsonArray and adds a reference at the end of the array.
  // It's a shortcut for JsonBuffer::createArray() and JsonArray::add()
  JsonArray &createNestedArray();

  // Creates a JsonObject and adds a reference at the end of the array.
  // It's a shortcut for JsonBuffer::createObject() and JsonArray::add()
  JsonObject &createNestedObject();

  // Removes element at specified index.
  void removeAt(size_t index);

  // Returns a reference an invalid JsonArray.
  // This object is meant to replace a NULL pointer.
  // This is used when memory allocation or JSON parsing fail.
  static JsonArray &invalid() { return _invalid; }

  // Serialize the array to the specified JsonWriter.
  void writeTo(Internals::JsonWriter &writer) const;

 private:
  node_type *getNodeAt(size_t index) const;

  template <typename TValue>
  bool setNodeAt(size_t index, TValue value);

  template <typename TValue>
  bool addNode(TValue);

  template <typename T>
  FORCE_INLINE bool setNodeValue(node_type *, T value);

  // The instance returned by JsonArray::invalid()
  static JsonArray _invalid;
};
}

#include "JsonArray.ipp"
