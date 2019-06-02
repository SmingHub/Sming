/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslValidator.cpp
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifdef ENABLE_SSL

#include "SslValidator.h"

static bool sslValidateCertificateSha1(SSL* ssl, void* data)
{
	uint8_t* hash = static_cast<uint8_t*>(data);
	bool success = false;
	if(hash != nullptr) {
		if(ssl != nullptr) {
			success = (ssl_match_fingerprint(ssl, hash) == 0);
		}
		delete[] hash;
	}

	return success;
}

static bool sslValidatePublicKeySha256(SSL* ssl, void* data)
{
	uint8_t* hash = static_cast<uint8_t*>(data);
	bool success = false;
	if(hash != nullptr) {
		if(ssl != nullptr) {
			success = (ssl_match_spki_sha256(ssl, hash) == 0);
		}
		delete[] hash;
	}

	return success;
}

/* SslValidatorList */

bool SslValidatorList::validate(SSL* ssl)
{
	if(ssl != nullptr && count() == 0) {
		// No validators specified, always succeed
		debug_d("SSL Validator: list empty, allow connection");
		return true;
	}

	/*
	 * We only need one match for a successful result, but we call all the validators anyway to
	 * ensure their data is released.
	 */
	bool success = false;
	for(unsigned i = 0; i < count(); i++) {
		auto& validator = operator[](i);
		// If we've already succeeded, then just release validator data without checking
		if(validator.callback(success ? nullptr : ssl, validator.data)) {
			debug_d("SSL validator: positive match");
			success = true;
		}
		// Callback will have released data so pointer no longer valid
		validator.data = nullptr;
	}

	if(ssl != nullptr && !success) {
		debug_d("SSL validator: NO match");
	}

	return success;
}

bool SslValidatorList::add(const uint8_t* fingerprint, SslFingerprintType type)
{
	SslValidatorCallback callback = nullptr;
	switch(type) {
	case eSFT_CertSha1:
		callback = sslValidateCertificateSha1;
		break;
	case eSFT_PkSha256:
		callback = sslValidatePublicKeySha256;
		break;
	default:
		debug_d("Unsupported SSL certificate fingerprint type");
	}

	if(callback == nullptr) {
		delete[] fingerprint;
		return false;
	}

	return add(callback, const_cast<uint8_t*>(fingerprint));
}

bool SslValidatorList::add(SslFingerprints& fingerprints)
{
	bool success = false;
	if(fingerprints.certSha1 != nullptr) {
		success = add(fingerprints.certSha1, eSFT_CertSha1);
		fingerprints.certSha1 = nullptr;
	}

	if(fingerprints.pkSha256 != nullptr) {
		success = add(fingerprints.pkSha256, eSFT_PkSha256);
		fingerprints.pkSha256 = nullptr;
	}

	return success;
}

#endif /* ENABLE_SSL */
