/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * JsonObjectStream.h
 *
 ****/

#pragma once

#include "MemoryDataStream.h"
#include "../ArduinoJson.h"

/** @brief JsonObject stream class
 * 	@ingroup    stream data
 *  @{
 *
 */

class JsonObjectStream : public MemoryDataStream
{
public:
	/** @brief Create a JSON object stream with a specific format
	 * 	@param format
	 * 	@param capacity Size of JSON buffer
    */
	JsonObjectStream(Json::SerializationFormat format, size_t capacity = 1024) : doc(capacity), format(format)
	{
	}

	/** @brief Create a JSON object stream using default (Compact) format
	 * 	@param capacity Size of JSON buffer
    */
	JsonObjectStream(size_t capacity = 1024) : doc(capacity)
	{
	}

	//Use base class documentation
	StreamType getStreamType() const override
	{
		return eSST_JsonObject;
	}

	/** @brief  Get the JSON root node
     *  @retval JsonObject Reference to the root node
     */
	JsonObject getRoot()
	{
		return doc.as<JsonObject>();
	}

	//Use base class documentation
	uint16_t readMemoryBlock(char* data, int bufSize) override;

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available() override
	{
		return (doc.isNull() ? 0 : Json::measure(doc, format));
	}

private:
	DynamicJsonDocument doc;
	Json::SerializationFormat format = Json::Compact;
	bool send = true;
};

/** @} */
