/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ArduinoJson.h - JSON/MesagePack support for Sming using ArduinoJson V6
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 * Provides ArduinoJson customisations to help simplify usage with Sming.
 *
 * 	- Add support for FlashString in keys and values
 * 	- Define enumeration for supported serialization formats (JSON/MessagePack)
 * 	- Serialization/de-serialization functions with additional FlashString and stream pointer support:
 * 		size_t Json::measure(source, format)
 * 		size_t Json::serialize(source, destination, format)
 * 		bool Json::deserialize(doc, input, format)
 * 		bool Json::deserialize(doc, input, size, format)
 * 		bool Json::loadFromFile(doc, filename, format)
 * 		bool Json::saveToFile(doc, filename, format)
 * 	- Support functions to simplify usage
 * 		bool Json::getValue(source&, dest&)
 *
 * ArduinoJson V6 vs V5
 * --------------------
 *
 *	See the migration guide for details: https://arduinojson.org/v6/doc/upgrade
 *
 *	Caution! If accessing a a key which doesn't exist:
 *				`json["key"].as<String>()` will return "null" (the serialised value)
 *				`json["key"].as<const char*>()` will return nullptr (same as ArduinoJson V5 `asString()` method)
 *
 ****/

#pragma once

#include "../ArduinoJson/src/ArduinoJson.h"
#include "FlashStringRefAdapter.hpp"
#include <Data/Stream/FileStream.h>

#ifndef JSON_ENABLE_COMPACT
#define JSON_ENABLE_COMPACT 1
#endif
#ifndef JSON_ENABLE_PRETTY
#define JSON_ENABLE_PRETTY 1
#endif
#ifndef JSON_ENABLE_MSGPACK
#define JSON_ENABLE_MSGPACK 1
#endif
#ifndef JSON_FORMAT_DEFAULT
#define JSON_FORMAT_DEFAULT Json::Compact
#endif

namespace Json
{
/**
 * @brief Copies a Json data value to a variable, but only if it exists
 * @param source Typically provided from JsonObject[key], JsonDocument[key] or JsonVariant[key] call
 * @param dest Variable to store value, unchanged if `data` is null
 * @retval bool true if value exists and was written to `value`
 * @note Use to perform existence check before assignment, without requiring
 * additional `containsKey` call or temporary variant. Example:
 *
 *		JsonDocument doc;
 *		...
 *		String value1;
 *		if(!Json::getValue(doc["key1"], value1)) {
 *			// Perform required action when value1 doesn't exist, such as initialising defaults
 *		}
 *
 * Where a simple default is required, use the | operator. Examples:
 *
 *		String value1 = doc["key1"].as<const char*> | "default value";
 *		int value2  = doc["key2] | -1;
 *
 */
template <typename TSource, typename TDest> bool getValue(const TSource& source, TDest& dest)
{
	if(source.isNull()) {
		return false;
	} else {
		dest = source.template as<TDest>();
		return true;
	}
}

/**
 * @brief Copies a Json data value to a variable, but only if it exists and its value has changed
 * @param source Typically provided from JsonObject[key], JsonDocument[key] or JsonVariant[key] call
 * @param dest Variable to store value, unchanged if `data` is null
 * @retval bool true if value exists and has changed, `value` updated
 */
template <typename TSource, typename TDest> bool getValueChanged(const TSource& source, TDest& dest)
{
	if(source.isNull()) {
		return false;
	}

	TDest value = source.template as<TDest>();
	if(value == dest) {
		return false; // value unchanged
	}

	dest = value;
	return true;
}

/**
 * @brief Describes format of serialized Json object
 */
enum SerializationFormat {
	Compact,	 ///< Compact JSON format
	Pretty,		 ///< Prettified JSON format, with spaces and line breaks
	MessagePack, ///< Message Pack (compact binary) format
};

inline SerializationFormat operator++(SerializationFormat& fmt)
{
	fmt = SerializationFormat(int(fmt) + 1);
	return fmt;
}

/**
 * @brief Compute the size of a serialized Json object for a specified format
 * @param source JsonDocument, JsonArray, JsonObject or JsonVariant
 * @param format Serialization format
 * @retval size_t Number of bytes that would be written when the document is serialized
 */
template <typename TSource> size_t measure(const TSource& source, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	switch(format) {
#if JSON_ENABLE_COMPACT
	case Compact:
		return measureJson(source);
#endif
#if JSON_ENABLE_PRETTY
	case Pretty:
		return measureJsonPretty(source);
#endif
#if JSON_ENABLE_MSGPACK
	case MessagePack:
		return measureMsgPack(source);
#endif
	default:
		return 0;
	}
}

/**
 * @brief Write a Json object in a specified format
 * @param source JsonDocument, JsonArray, JsonObject or JsonVariant
 * @param destination Where to write output (char[], Print& or String&)
 * @param format Serialization format to use when writing
 * @retval size_t The number of bytes written
 */
template <typename TSource, typename TDestination>
size_t serialize(const TSource& source, TDestination& destination, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	switch(format) {
#if JSON_ENABLE_COMPACT
	case Compact:
		return serializeJson(source, destination);
#endif
#if JSON_ENABLE_PRETTY
	case Pretty:
		return serializeJsonPretty(source, destination);
#endif
#if JSON_ENABLE_MSGPACK
	case MessagePack:
		return serializeMsgPack(source, destination);
#endif
	default:
		return 0;
	}
}

/**
 * @brief Write a Json object in a specified format
 * @param source JsonDocument, JsonArray, JsonObject or JsonVariant
 * @param destination Where to write output (Print*)
 * @param format Serialization format to use when writing
 * @retval size_t The number of bytes written
 * @note This variant provides support for stream pointers, with null check
 */
template <typename TSource, typename TPrint>
typename std::enable_if<std::is_base_of<Print, TPrint>::value, size_t>::type
serialize(const TSource& source, TPrint* destination, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	return (destination == nullptr) ? 0 : serialize(source, *destination, format);
}

/**
 * @brief Write a Json object in a specified format
 * @param source JsonDocument, JsonArray, JsonObject or JsonVariant
 * @param buffer Buffer for output data
 * @param bufferSize Size of buffer
 * @param format Serialization format to use when writing
 * @retval size_t The number of bytes written
 */
template <typename TSource>
size_t serialize(const TSource& source, char* buffer, size_t bufferSize,
				 SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	switch(format) {
#if JSON_ENABLE_COMPACT
	case Compact:
		return serializeJson(source, buffer, bufferSize);
#endif
#if JSON_ENABLE_PRETTY
	case Pretty:
		return serializeJsonPretty(source, buffer, bufferSize);
#endif
#if JSON_ENABLE_MSGPACK
	case MessagePack:
		return serializeMsgPack(source, buffer, bufferSize);
#endif
	default:
		return 0;
	}
}

/**
 * @brief Serialize a Json object in a specified format, returning it in a String object
 * @param source JsonDocument, JsonArray, JsonObject or JsonVariant
 * @param format Serialization format to use when writing
 * @retval String The serialized data
 */
template <typename TSource> String serialize(const TSource& source, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	String result;
	serialize(source, result, format);
	return result;
}

/**
 * @brief Save a Json object to a file in a specified format
 * @param source JsonDocument, JsonArray, JsonObject, JsonVariant
 * @param filename Name of file to create, will always be overwritten
 * @param format Serialization format to use when writing
 * @retval bool true on success, false if the write failed
 */
template <typename TSource>
bool saveToFile(const TSource& source, const String& filename, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	FileStream stream(filename, eFO_WriteOnly | eFO_CreateNewAlways);
	if(!stream.isValid()) {
		return false;
	}

	if(serialize(source, stream, format) == 0) {
		return false;
	}

	return stream.getLastError() == 0;
}

template <typename TInput>
bool deserializeInternal(JsonDocument& doc, TInput& input, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	switch(format) {
#if JSON_ENABLE_COMPACT || JSON_ENABLE_PRETTY
	case Compact:
	case Pretty:
		return deserializeJson(doc, input) == DeserializationError::Ok;
#endif
#if JSON_ENABLE_MSGPACK
	case MessagePack:
		return deserializeMsgPack(doc, input) == DeserializationError::Ok;
#endif
	default:
		return false;
	}
}

/**
 * @brief Read a JsonDocument from formatted data
 * @param doc Document to store the decoded file
 * @param input Where to get data from (see notes)
 * @param format Format of the data
 * @retval bool true on success, false on error
 * @note Supported read-only input types:
 *
 * 		Stream&
 * 		String&
 * 		const String&
 * 		const FlashString&
 *
 * @note `String&` is always read-only. If you wish to deserialize content in-situ, use:
 *
 * 		Json:: deserialize(doc, str.begin(), str.length(), format)
 *
 * Don't forget to keep `str` in scope until you're finished with `doc`.
 */
template <typename TInput>
bool deserialize(JsonDocument& doc, TInput& input, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	return deserializeInternal(doc, input, format);
}

/**
 * @brief Read a JsonDocument from formatted data
 * @param doc Document to store the decoded file
 * @param input Where to get data from (see notes)
 * @param format Format of the data
 * @retval bool true on success, false on error
 * @note Supported writeable input types (content will be modified in-situ):
 *
 * 		char*
 *
 * @note Supported read-only input types:
 *
 * 		const char*
 * 		const __FlashStringHelper*
 * 		const FlashString*
 */
template <typename TInput>
typename std::enable_if<!std::is_base_of<Stream, TInput>::value, bool>::type
deserialize(JsonDocument& doc, TInput* input, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	return deserializeInternal(doc, input, format);
}

/**
 * @brief Read a JsonDocument from formatted data
 * @param doc Document to store the decoded file
 * @param input Where to get data from (see notes)
 * @param format Format of the data
 * @retval bool true on success, false on error
 * @note Supports stream pointers (Stream and inherited classses)
 * This function can be safely used with null pointers
 */
template <typename TStream>
typename std::enable_if<std::is_base_of<Stream, TStream>::value, bool>::type
deserialize(JsonDocument& doc, TStream* input, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	return (input == nullptr) ? false : deserializeInternal(doc, *input, format);
}

/**
 * @brief Read a JsonDocument from formatted data
 * @param doc Document to store the decoded file
 * @param input Where to get data from (see notes)
 * @param inputSize Maximum number of bytes to read from `input`
 * @param format Format of the data
 * @retval bool true on success, false on error
 * @note Supported writeable input types (content will be modified in-situ):
 *
 * 		char*, size_t
 *
 * @note Supported read-only input types:
 *
 * 		const char*, size_t
 * 		const __FlashStringHelper*, size_t
 */
template <typename TInput>
bool deserialize(JsonDocument& doc, TInput* input, size_t inputSize, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	switch(format) {
#if JSON_ENABLE_COMPACT || JSON_ENABLE_PRETTY
	case Compact:
	case Pretty:
		return deserializeJson(doc, input, inputSize) == DeserializationError::Ok;
#endif
#if JSON_ENABLE_MSGPACK
	case MessagePack:
		return deserializeMsgPack(doc, input, inputSize) == DeserializationError::Ok;
#endif
	default:
		return false;
	}
}

/**
 * @brief Parses the contents of a serialized file into a JsonDocument object
 * @param doc Document to store the decoded file
 * @param filename Name of file to create, will always be overwritten
 * @param format Format of the data to be parsed
 * @retval bool true on success, false if the file couldn't be read or there was a parsing error
 */
inline bool loadFromFile(JsonDocument& doc, const String& filename, SerializationFormat format = JSON_FORMAT_DEFAULT)
{
	FileStream stream(filename);
	return stream.isValid() ? deserialize(doc, stream, format) : false;
}

} // namespace Json
