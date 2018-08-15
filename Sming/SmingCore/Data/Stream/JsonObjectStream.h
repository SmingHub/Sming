/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_JSONOBJECTSTREAM_H_
#define _SMING_CORE_DATA_JSONOBJECTSTREAM_H_

#include "MemoryDataStream.h"
#include "../Libraries/ArduinoJson/include/ArduinoJson.h"

/** @brief JsonObject stream class
 * 	@ingroup    stream data
 *  @{
 *
 */

class JsonObjectStream : public MemoryDataStream {
public:
	/** @brief  Create a JSON object stream
    */
	JsonObjectStream() : _rootNode(_buffer.createObject())
	{}

	virtual ~JsonObjectStream()
	{}

	//Use base class documentation
	virtual StreamType getStreamType() const
	{
		return eSST_JsonObject;
	}

	/** @brief  Get the JSON root node
     *  @retval JsonObject Reference to the root node
     */
	JsonObject& getRoot()
	{
		return _rootNode;
	}

	//Use base class documentation
	virtual size_t readMemoryBlock(char* data, size_t bufSize)
	{
		if (_send && _rootNode.success()) {
			_rootNode.printTo(*this);
			_send = false;
		}

		return MemoryDataStream::readMemoryBlock(data, bufSize);
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available()
	{
		return _rootNode.success() ? _rootNode.measureLength() : 0;
	}

private:
	DynamicJsonBuffer _buffer;
	JsonObject& _rootNode;
	bool _send = true;
};

/** @} */
#endif /* _SMING_CORE_DATA_JSONOBJECTSTREAM_H_ */
