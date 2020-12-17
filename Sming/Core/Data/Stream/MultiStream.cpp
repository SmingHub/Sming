/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MultiStream.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "MultiStream.h"

uint16_t MultiStream::readMemoryBlock(char* data, int bufSize)
{
	if(stream != nullptr && stream->isFinished()) {
		delete stream;
		stream = nullptr;
	}

	if(stream == nullptr) {
		stream = getNextStream();
		if(stream == nullptr) {
			finished = true;
			return 0;
		}
	}

	return stream->readMemoryBlock(data, bufSize);
}

bool MultiStream::seek(int len)
{
	return stream ? stream->seek(len) : false;
}
