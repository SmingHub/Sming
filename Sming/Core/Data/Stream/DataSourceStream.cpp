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
