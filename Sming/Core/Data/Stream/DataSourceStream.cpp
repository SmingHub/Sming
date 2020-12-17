/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DataSourceStream.cpp
 *
 ****/

#include "DataSourceStream.h"

int IDataSourceStream::read()
{
	int res = peek();
	if(res != -1) {
		seek(1);
	}

	return res;
}

int IDataSourceStream::peek()
{
	char c;
	if(readMemoryBlock(&c, 1) == 1) {
		return (int)c;
	}

	return -1;
}

size_t IDataSourceStream::readBytes(char* buffer, size_t length)
{
	auto count = readMemoryBlock(buffer, length);
	if(count > 0) {
		seek(count);
	}
	return count;
}

String IDataSourceStream::readString(size_t maxLen)
{
	size_t avail = available();
	size_t len = std::min(avail, maxLen);

	String s;
	if(s.setLength(len)) {
		len = readBytes(s.begin(), len);
		s.setLength(len);
	}
	return s;
}
