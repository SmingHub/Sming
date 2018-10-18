/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "EndlessMemoryStream.h"

EndlessMemoryStream::~EndlessMemoryStream()
{
	delete stream;
	stream = NULL;
}

StreamType EndlessMemoryStream::getStreamType()
{
	return eSST_Memory;
}

uint16_t EndlessMemoryStream::readMemoryBlock(char* data, int bufSize)
{
	if(stream == NULL) {
		return 0;
	}

	return stream->readMemoryBlock(data, bufSize);
}

//Use base class documentation
bool EndlessMemoryStream::seek(int len)
{
	if(stream == NULL) {
		return false;
	}

	int res = stream->seek(len);
	if(stream->isFinished()) {
		delete stream;
		stream = NULL;
	}

	return res;
}

size_t EndlessMemoryStream::write(uint8_t charToWrite)
{
	if(stream == NULL) {
		stream = new MemoryDataStream();
	}

	return stream->write(charToWrite);
}

size_t EndlessMemoryStream::write(const uint8_t* buffer, size_t size)
{
	if(stream == NULL) {
		stream = new MemoryDataStream();
	}

	return stream->write(buffer, size);
}

bool EndlessMemoryStream::isFinished()
{
	return false;
}
