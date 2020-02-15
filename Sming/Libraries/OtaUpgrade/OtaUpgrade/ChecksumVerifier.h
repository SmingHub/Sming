/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ChecksumVerifier.h
 *
 ****/

#pragma once

#include <Crypto/Md5.h>

namespace OtaUpgrade
{
/**
 * @brief Checksum verifier used by `BasicStream` if signature verification is disabled.
 *
 * Wrap `Crypto::Md5` such that it provides an interface compatible to `SignatureVerifier`.
 */
class ChecksumVerifier : public Crypto::Md5
{
public:
	typedef Hash VerificationData; ///< Checksum type

	/** Verify the given \c checksum.
	 * @return `true` if checksum matches content, `false` otherwise.
	 */
	bool verify(const VerificationData& checksum)
	{
		return (getHash() == checksum);
	}
};

} // namespace OtaUpgrade
