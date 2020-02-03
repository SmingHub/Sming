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

IDataSourceStream* MultipartStream::getNextStream()
{
	// Return content, if available
	if(bodyPart.stream != nullptr) {
		auto stream = bodyPart.stream;
		bodyPart.stream = nullptr;
		return stream;
	}

	// Fetch next part to send
	bodyPart = producer();

	// Generate header fragment
	auto stream = new MemoryDataStream();
	stream->ensureCapacity(4 + 16 + 4);
	stream->print("\r\n");
	stream->print("--");
	stream->print(getBoundary());
	if(bodyPart.headers == nullptr) {
		// No more parts
		stream->print("--");
	}
	stream->print("\r\n");

	if(bodyPart.headers != nullptr) {
		if(bodyPart.stream != nullptr && !bodyPart.headers->contains(HTTP_HEADER_CONTENT_LENGTH)) {
			auto avail = bodyPart.stream->available();
			if(avail >= 0) {
				(*bodyPart.headers)[HTTP_HEADER_CONTENT_LENGTH] = avail;
			}
		}

		for(unsigned i = 0; i < bodyPart.headers->count(); i++) {
			stream->print((*bodyPart.headers)[i]);
		}

		delete bodyPart.headers;
		bodyPart.headers = nullptr;

		stream->print("\r\n");
	}

	return stream;
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
			boundary[i] = pool[os_random() % (sizeof(__pstr__pool) - 1)];
		}
		boundary[len - 1] = 0;
	}

	return boundary;
}
