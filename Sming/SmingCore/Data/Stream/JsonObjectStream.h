/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_JSON_OBJECT_STREAM_H_
#define _SMING_CORE_DATA_JSON_OBJECT_STREAM_H_

#include "MemoryDataStream.h"
#include "../Libraries/ArduinoJson/include/ArduinoJson.h"

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

	virtual ~JsonObjectStream()
	{
	}

	//Use base class documentation
	virtual StreamType getStreamType()
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
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available()
	{
		return rootNode.success() ? rootNode.measureLength() : 0;
	}

private:
	DynamicJsonBuffer buffer;
	JsonObject& rootNode;
	bool send = true;
};

/** @} */
#endif /* _SMING_CORE_DATA_JSON_OBJECT_STREAM_H_ */
