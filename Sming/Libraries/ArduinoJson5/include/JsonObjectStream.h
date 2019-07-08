/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * JsonObjectStream.h - Stream implementation for ArduinoJson Version 5
 *
 ****/

#pragma once

#include <Data/Stream/MemoryDataStream.h>
#include "ArduinoJson.h"

/** @brief JsonObject stream class
 * 	@ingroup    stream data
 *  @{
 *
 */

class JsonObjectStream : public MemoryDataStream
{
public:
	/** @brief  Create a JSON object stream
    */
	JsonObjectStream() : rootNode(buffer.createObject())
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
	JsonObject& getRoot()
	{
		return rootNode;
	}

	//Use base class documentation
	uint16_t readMemoryBlock(char* data, int bufSize) override;

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available() override
	{
		return rootNode.success() ? rootNode.measureLength() : 0;
	}

private:
	DynamicJsonBuffer buffer;
	JsonObject& rootNode;
	bool send = true;
};

/** @} */
