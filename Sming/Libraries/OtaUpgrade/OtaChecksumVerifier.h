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
#include <array>

/**
 * @brief Checksum verifier used by `BasicOtaUpgradeStream` if signature verification is disabled.
 *
 * This is a simple C++ wrapper for the MD5 ROM functions.
 */
class OtaChecksumVerifier
{
public:
	typedef std::array<uint8_t, MD5_SIZE> VerificationData; ///< Checksum type

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
	bool verify(const VerificationData& checksum)
	{
		VerificationData expectedChecksum;
		MD5Final(expectedChecksum.data(), &context);
		return (checksum == expectedChecksum);
	}

private:
	MD5Context context;
};
