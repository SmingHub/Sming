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

/**
 * @brief Checksum verifier used by `BasicOtaUpgradeStream` if signature verification is disabled.
 *
 * This is a simple C++ wrapper for the MD5 ROM functions.
 */
class OtaChecksumVerifier
{
	MD5Context context;

public:
	static const size_t NumBytes = MD5_SIZE; ///< Size of checksum in bytes.

	OtaChecksumVerifier()
	{
		MD5Init(&context);
	}

	/** Continue MD5 checksum calculation for the given chunk of data.
	 */
	void update(const void* data, size_t size)
	{
		MD5Update(&context, data, size);
	}

	/** Verify given \c checksum.
	 * @return `true` if checksum and content matches, `false` otherwise.
	 */
	bool verify(const uint8_t (&checksum)[MD5_SIZE])
	{
		uint8_t expectedChecksum[MD5_SIZE];
		MD5Final(expectedChecksum, &context);
		return (memcmp(expectedChecksum, checksum, MD5_SIZE) == 0);
	}
};
