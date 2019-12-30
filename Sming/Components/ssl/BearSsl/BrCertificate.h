/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * X509Context.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/Certificate.h>
#include <Network/Ssl/Crypto.h>

namespace Ssl
{
class BrCertificate : public Ssl::Certificate
{
public:
	uint8_t sha1Hash[SHA1_SIZE];

	bool matchFingerprint(const uint8_t* hash) const override
	{
		return memcmp(hash, sha1Hash, SHA1_SIZE) == 0;
	}

	bool matchPki(const uint8_t* hash) const override
	{
		return false;
	}

	const String getName(Name name) const override
	{
		return nullptr;
	}
};

} // namespace Ssl
