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
	}

	return success;
}

bool sslValidatePublicKeySha256(SSL* ssl, void* data)
{
	uint8_t* hash = static_cast<uint8_t*>(data);
	bool success = false;
	if(hash != nullptr) {
		success = (ssl_match_spki_sha256(ssl, hash) == 0);
	}

	return success;
}

/* SSLValidatorList */

void SSLValidatorList::clear()
{
	// Ensure any remaining fingerprint data is released
	for(unsigned i = 0; i < count(); ++i) {
		delete[] static_cast<uint8_t*>(elementAt(i).data);
	}
	Vector::clear();
}

bool SSLValidatorList::validate(SSL* ssl)
{
	if(count() == 0) {
		// No validators specified, always succeed
		debug_d("SSL Validator: list empty, allow connection");
		return true;
	}

	// Need a match against a fingerprint
	bool success = false;
	for(unsigned i = 0; i < count(); i++) {
		auto& validator = elementAt(i);
		if(validator.callback(ssl, validator.data)) {
			debug_d("SSL validator: positive match");
			success = true;
			break;
		}
	}

	if(!success) {
		debug_d("SSL validator: NO match");
	}

	return success;
}

#endif /* ENABLE_SSL */
