/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MultipartStream.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "MultipartStream.h"
#include "MemoryDataStream.h"

bool MultipartStream::onCompleted()
{
	auto stream = new MemoryDataStream();
	stream->ensureCapacity(4 + 16 + 4);
	stream->print(_F("\r\n--"));
	stream->print(getBoundary());
	stream->print(_F("--\r\n"));
	return true;
}

void MultipartStream::onNextStream()
{
	auto stream = new MemoryDataStream();
	stream->ensureCapacity(4 + 16 + 4);
	stream->print(_F("\r\n--"));
	stream->print(getBoundary());
	stream->print("\r\n");

	if(result.headers != nullptr) {
		if(!result.headers->contains(HTTP_HEADER_CONTENT_LENGTH)) {
			if(result.stream != nullptr && result.stream->available() >= 0) {
				(*result.headers)[HTTP_HEADER_CONTENT_LENGTH] = result.stream->available();
			}
		}

		for(unsigned i = 0; i < result.headers->count(); i++) {
			stream->print((*result.headers)[i]);
		}

		delete result.headers;
		result.headers = nullptr;
	}
	stream->print("\r\n");

	nextStream = result.stream;
}

const char* MultipartStream::getBoundary()
{
	if(boundary[0] == 0) {
		PSTR_ARRAY(pool, "0123456789"
						 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						 "abcdefghijklmnopqrstuvwxyz");

		int len = sizeof(boundary);
		memset(boundary, 0, len);
		for(int i = 0; i < len - 1; ++i) {
			boundary[i] = pool[os_random() % (sizeof(PSTR_pool) - 1)];
		}
		boundary[len - 1] = 0;
	}

	return boundary;
}
