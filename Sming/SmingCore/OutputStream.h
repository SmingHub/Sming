/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 *
 * Authors: 2017-... Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_OUTPUTSTREAM_H_
#define _SMING_CORE_OUTPUTSTREAM_H_

#include <user_config.h>
#include "../../Wiring/WString.h"
#include "../SmingCore/FileSystem.h"

class IOutputStream
{
public:
	virtual ~IOutputStream() {}

	virtual size_t write(const uint8_t* data, size_t size) = 0;
	virtual bool close() = 0;
};

class FileOutputStream: public IOutputStream
{
public:
	FileOutputStream(String filename, FileOpenFlags flags = eFO_CreateNewAlways | eFO_WriteOnly);

	virtual size_t write(const uint8_t* data, size_t size);

	virtual bool close();

	~FileOutputStream();

private:
	file_t handle;
};

#endif /* _SMING_CORE_OUTPUTSTREAM_H_ */
