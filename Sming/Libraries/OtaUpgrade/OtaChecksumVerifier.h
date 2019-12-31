/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OtaChecksumVerifier.h
 *
 ****/

#pragma once

#include "romfunc_md5.h"
#include <string.h>

// simple wrapper for MD5 C API
class OtaChecksumVerifier
{
	MD5Context context;

public:
	static const size_t NumBytes = MD5_SIZE;

	OtaChecksumVerifier()
	{
		MD5Init(&context);
	}
	void update(const void* data, size_t size)
	{
		MD5Update(&context, data, size);
	}

	bool verify(const uint8_t (&checksum)[MD5_SIZE])
	{
		uint8_t expectedChecksum[MD5_SIZE];
		MD5Final(expectedChecksum, &context);
		return (memcmp(expectedChecksum, checksum, MD5_SIZE) == 0);
	}
};
