/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "SslValidator.h"

#ifdef ENABLE_SSL

bool sslValidateCertificateSha1(const SSL* ssl, const void* data)
{
	auto hash = (const uint8_t*)data;
	if (!hash)
		return false;

	bool match = (ssl_match_fingerprint(ssl, hash) == 0);
	delete[] hash;

	return match;
}

bool sslValidatePublicKeySha256(const SSL* ssl, const void* data)
{
	auto hash = (const uint8_t*)data;
	if (!hash)
		return false;

	bool match = (ssl_match_spki_sha256(ssl, hash) == 0);
	delete[] hash;

	return match;
}

#endif /* ENABLE_SSL */
