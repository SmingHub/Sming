// Copyright Benoit Blanchon 2014-2015
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson

#include "../include/ArduinoJson/JsonObject.hpp"

#include <string.h>  // for strcmp

#include "../include/ArduinoJson/Internals/StringBuilder.hpp"
#include "../include/ArduinoJson/JsonArray.hpp"
#include "../include/ArduinoJson/JsonBuffer.hpp"
#include "../../SmingCore/DataSourceStream.h"

using namespace ArduinoJson;
using namespace ArduinoJson::Internals;

JsonObject JsonObject::_invalid(NULL);

JsonVariant &JsonObject::at(key_type key) {
  node_type *node = getNodeAt(key);
  return node ? node->content.value : JsonVariant::invalid();
}

JsonVariant &JsonObject::operator[](key_type key) {
  // try to find an existing node
  node_type *node = getNodeAt(key);

  // not fount => create a new one
  if (!node) {
    node = createNode();
    if (!node) return JsonVariant::invalid();

    node->content.key = key;
    addNode(node);
  }

  return node->content.value;
}

JsonVariant &JsonObject::operator[](const String& stringKey) {
	auto& id = _buffer->createStringStorage(stringKey);
	return (*this)[id.c_str()];
}

void JsonObject::remove(key_type key) { removeNode(getNodeAt(key)); }

JsonArray &JsonObject::createNestedArray(key_type key) {
  if (!_buffer) return JsonArray::invalid();
  JsonArray &array = _buffer->createArray();
  add(key, array);
  return array;
}

JsonObject &JsonObject::createNestedObject(key_type key) {
  if (!_buffer) return JsonObject::invalid();
  JsonObject &object = _buffer->createObject();
  add(key, object);
  return object;
}

JsonArray &JsonObject::createNestedArray(const String& stringKey) {
  if (!_buffer) return JsonArray::invalid();
  JsonArray &array = _buffer->createArray();
  add(stringKey, array);
  return array;
}

JsonObject &JsonObject::createNestedObject(const String& stringKey) {
  if (!_buffer) return JsonObject::invalid();
  JsonObject &object = _buffer->createObject();
  add(stringKey, object);
  return object;
}

JsonObject::node_type *JsonObject::getNodeAt(key_type key) const {
  for (node_type *node = _firstNode; node; node = node->next) {
    if (!strcmp(node->content.key, key)) return node;
  }
  return NULL;
}

JsonVariant &JsonObject::add(const String& stringKey) {
	auto& id = _buffer->createStringStorage(stringKey);
	return add(id.c_str());
}

void JsonObject::addCopy(key_type key, const String &stringVal)
{
	auto& val = _buffer->createStringStorage(stringVal);
	return add(key, val.c_str());
}

void JsonObject::addCopy(const String& stringKey, const String &stringVal)
{
	auto& id = _buffer->createStringStorage(stringKey);
	auto& val = _buffer->createStringStorage(stringVal);
	return add(id.c_str(), val.c_str());
}

void JsonObject::writeTo(JsonWriter &writer) const {
  writer.beginObject();

  const node_type *node = _firstNode;
  while (node) {
    writer.writeString(node->content.key);
    writer.writeColon();
    node->content.value.writeTo(writer);

    node = node->next;
    if (!node) break;

    writer.writeComma();
  }

  writer.endObject();
}

String JsonObject::toJsonString(bool prettyPrintStyle /* = true*/) const
{
  String result;
  MemoryDataStream stream;
  if (prettyPrintStyle)
	  prettyPrintTo(stream);
  else
	  printTo(stream);

  result.setString(stream.getStreamPointer(), stream.getStreamLength());
  return result;
}
