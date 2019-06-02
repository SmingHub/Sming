/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * JsonObjectStream.cpp
 *
 ****/

#include "JsonObjectStream.h"

uint16_t JsonObjectStream::readMemoryBlock(char* data, int bufSize)
{
	if(send && !doc.isNull()) {
		Json::serialize(doc, this, format);
		send = false;
	}

	return MemoryDataStream::readMemoryBlock(data, bufSize);
}
