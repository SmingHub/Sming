/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "MultiStream.h"

MultiStream::~MultiStream()
{
	delete stream;
	stream = nullptr;
	delete nextStream;
	nextStream = nullptr;
}

size_t MultiStream::write(uint8_t charToWrite)
{
	// those methods should not be used...
	return 0;
}

size_t MultiStream::write(const uint8_t* buffer, size_t size)
{
	// those methods should not be used...
	return 0;
}

//Use base class documentation
uint16_t MultiStream::readMemoryBlock(char* data, int bufSize)
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
		nextStream = getNextStream();
		if(!nextStream) {
			finished = true;
			if(!onCompleted()) {
				return 0;
			}
		} else {
			onNextStream();
		}
	}

	return stream->readMemoryBlock(data, bufSize);
}

bool MultiStream::seek(int len)
{
	if(!stream) {
		return false;
	}

	return stream->seek(len);
}

bool MultiStream::isFinished()
{
	return (finished && (stream == nullptr || stream->isFinished()));
}
