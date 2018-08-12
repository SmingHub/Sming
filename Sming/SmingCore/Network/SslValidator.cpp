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

bool sslValidateCertificateSha1(SSL* ssl, void* data)
{
	uint8_t* hash = (uint8_t*)data;
	bool success = false;
	if (hash != NULL) {
		success = (ssl_match_fingerprint(ssl, hash) == 0);
		delete[] hash;
	}

	return success;
}

bool sslValidatePublicKeySha256(SSL* ssl, void* data)
{
	uint8_t* hash = (uint8_t*)data;
	bool success = false;
	if (hash != NULL) {
		success = (ssl_match_spki_sha256(ssl, hash) == 0);
		delete[] hash;
	}

	return success;
}

#endif /* ENABLE_SSL */
