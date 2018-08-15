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
size_t MultipartStream::readMemoryBlock(char* data, size_t bufSize)
{
	if (_stream && _stream->isFinished()) {
		delete _stream;
		_stream = nullptr;
	}

	if (!_stream && _nextStream) {
		_stream = _nextStream;
		_nextStream = nullptr;
	}

	if (!_stream) {
		HttpPartResult result = _producer();

		auto mem = new MemoryDataStream();

		// Sending a result without stream is the way to stop the "production"
		if (!result.stream) {
			mem->print(_F("\r\n--"));
			mem->print(getBoundary());
			mem->print(_F("--\r\n"));
			_finished = true;
		}
		else {
			mem->print(_F("\r\n--"));
			mem->print(getBoundary());
			mem->print("\r\n");
			if (result.headers) {
				if (!result.headers->contains(hhfn_ContentLength)) {
					if (result.stream && result.stream->available() >= 0)
						(*result.headers)[hhfn_ContentLength] = result.stream->available();
				}

				for (unsigned i = 0; i < result.headers->count(); i++)
					mem->print((*result.headers)[i]);

				delete result.headers;
				result.headers = nullptr;
			}
			mem->print("\r\n");

			_stream = mem;

			_nextStream = result.stream;
		}
	}

	return _stream->readMemoryBlock(data, bufSize);
}

const char* MultipartStream::getBoundary()
{
	if (_boundary[0] == 0) {
		PSTR_ARRAY(pool, "0123456789"
						 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						 "abcdefghijklmnopqrstuvwxyz");

		unsigned len = sizeof(_boundary);
		memset(_boundary, 0, len);
		for (unsigned i = 0; i < len - 1; ++i)
			_boundary[i] = pool[os_random() % (sizeof(pool) - 1)];
		_boundary[len - 1] = 0;
	}

	return _boundary;
}
