/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "OutputStream.h"

FileOutputStream::FileOutputStream(String fileName, FileOpenFlags flags /* = eFO_CreateNewAlways | eFO_WriteOnly */) {
	handle = fileOpen(fileName, flags);
}

size_t FileOutputStream::write(const uint8_t* data, size_t size) {
	if(handle < 1) {
		return -1; // report problems..
	}

	return fileWrite(handle, data, size);
}

bool FileOutputStream::close() {
	if(handle < 1) {
		return false; // report problems..
	}

	fileClose(handle);
	handle = 0;
	return true;
}

FileOutputStream::~FileOutputStream()
{
	close();
}
