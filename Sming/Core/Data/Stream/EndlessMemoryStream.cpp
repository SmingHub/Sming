/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EndlessMemoryStream.cpp
 *
 ****/

#include "EndlessMemoryStream.h"

bool EndlessMemoryStream::seek(int len)
{
	if(stream == nullptr) {
		return false;
	}

	int res = stream->seek(len);
	if(stream->isFinished()) {
		delete stream;
		stream = nullptr;
	}

	return res;
}

size_t EndlessMemoryStream::write(const uint8_t* buffer, size_t size)
{
	if(stream == nullptr) {
		stream = new MemoryDataStream();
	}

	return stream->write(buffer, size);
}
