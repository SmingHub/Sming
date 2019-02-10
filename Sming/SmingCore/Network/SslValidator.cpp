/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifdef ENABLE_SSL

#include "SslValidator.h"

bool sslValidateCertificateSha1(SSL* ssl, void* data)
{
	uint8_t* hash = static_cast<uint8_t*>(data);
	bool success = false;
	if(hash != nullptr) {
		success = (ssl_match_fingerprint(ssl, hash) == 0);
		delete[] hash;
	}

	return success;
}

bool sslValidatePublicKeySha256(SSL* ssl, void* data)
{
	uint8_t* hash = static_cast<uint8_t*>(data);
	bool success = false;
	if(hash != nullptr) {
		success = (ssl_match_spki_sha256(ssl, hash) == 0);
		delete[] hash;
	}

	return success;
}

/* SSLValidatorList */

SSLValidatorList::~SSLValidatorList()
{
	// Ensure any remaining fingerprint data is released
	for(unsigned i = 0; i < count(); ++i) {
		delete[] static_cast<uint8_t*>(elementAt(i).data);
	}
}

bool SSLValidatorList::validate(SSL* ssl)
{
	if(count() == 0) {
		// No validators specified, always succeed
		return true;
	}

	// Need a match against a fingerprint
	for(unsigned i = 0; i < count(); i++) {
		auto& validator = elementAt(i);
		if(validator.callback(ssl, validator.data)) {
			return true;
		}
	}

	return false;
}

#endif /* ENABLE_SSL */
