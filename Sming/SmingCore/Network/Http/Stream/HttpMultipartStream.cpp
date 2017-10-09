/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "HttpMultipartStream.h"

HttpMultipartStream::HttpMultipartStream(HttpPartProducerDelegate delegate)
{
	this->producer = delegate;
}


HttpMultipartStream::~HttpMultipartStream()
{
	delete stream;
	stream = NULL;
	delete nextStream;
	nextStream = NULL;
}

size_t HttpMultipartStream::write(uint8_t charToWrite)
{
	// TODO: those methods should not be used...
	return 0;
}

size_t HttpMultipartStream::write(const uint8_t *buffer, size_t size)
{
	// TODO: those methods should not be used...
	return 0;
}
	//Use base class documentation
uint16_t HttpMultipartStream::readMemoryBlock(char* data, int bufSize)
{
	if(stream != NULL && stream->isFinished()) {
		delete stream;
		stream = NULL;
	}

	if(stream == NULL && nextStream != NULL) {
		stream = nextStream;
		nextStream = NULL;
	}

	if(stream == NULL) {
		HttpPartResult result = producer();
		stream = new MemoryDataStream();

		String line = String("\r\n--") + getBoundary() + String("\r\n");
		stream->write((uint8_t *)line.c_str(), line.length());
		if(result.headers != NULL) {

			if(!result.headers->contains("Content-Length") ) {
				if(result.stream != NULL && result.stream->length() > -1) {
					(*result.headers)["Content-Length"] = result.stream->length();
				}
			}

			for (int i = 0; i < result.headers->count(); i++) {
				line = result.headers->keyAt(i) + ": " + result.headers->valueAt(i) + "\r\n";
				stream->write((uint8_t *)line.c_str(), line.length());
			}

			delete result.headers;
			result.headers = NULL;
		}
		line = "\r\n";
		stream->write((uint8_t *)line.c_str(), line.length());

		nextStream = result.stream;
	}

	return stream->readMemoryBlock(data, bufSize);
}

bool HttpMultipartStream::seek(int len)
{
	return stream->seek(len);
}

bool HttpMultipartStream::isFinished()
{
	return false;
}

const char* HttpMultipartStream::getBoundary()
{
	if(boundary[0] == 0) {
		static const char pool[] =
		        "0123456789"
		        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		        "abcdefghijklmnopqrstuvwxyz";

		int len = sizeof(boundary);
		memset(boundary, 0, len);
		for (int i = 0; i < len - 1; ++i) {
			boundary[i] = pool[os_random() % (sizeof(pool) - 1)];
		}
		boundary[len - 1] = 0;
	}

	return boundary;
}
