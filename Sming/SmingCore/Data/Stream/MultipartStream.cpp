/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "MultipartStream.h"
#include "MemoryDataStream.h"

MultipartStream::MultipartStream(HttpPartProducerDelegate delegate) : producer(delegate)
{
}

MultipartStream::~MultipartStream()
{
	delete stream;
	stream = nullptr;
	delete nextStream;
	nextStream = nullptr;
}

size_t MultipartStream::write(uint8_t charToWrite)
{
	// TODO: those methods should not be used...
	return 0;
}

size_t MultipartStream::write(const uint8_t* buffer, size_t size)
{
	// TODO: those methods should not be used...
	return 0;
}
//Use base class documentation
uint16_t MultipartStream::readMemoryBlock(char* data, int bufSize)
{
	if(stream != nullptr && stream->isFinished()) {
		delete stream;
		stream = nullptr;
	}

	if(stream == nullptr && nextStream != nullptr) {
		stream = nextStream;
		nextStream = nullptr;
	}

	if(stream == nullptr) {
		HttpPartResult result = producer();

		stream = new MemoryDataStream();

		if(result.stream == nullptr) { // Sending a result without stream is the way to stop the "production"
			String line = F("\r\n--") + getBoundary() + F("--\r\n");
			stream->print(line);

			finished = true;
		} else {
			String line = F("\r\n--") + getBoundary() + "\r\n";
			stream->print(line);
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
	}

	return stream->readMemoryBlock(data, bufSize);
}

bool MultipartStream::seek(int len)
{
	return stream->seek(len);
}

bool MultipartStream::isFinished()
{
	return (finished && (stream == nullptr || stream->isFinished()));
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
			boundary[i] = pool[os_random() % (sizeof(pool) - 1)];
		}
		boundary[len - 1] = 0;
	}

	return boundary;
}
