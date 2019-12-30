/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Validator.cpp
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include <Network/Ssl/Validator.h>
#include <debug_progmem.h>

namespace Ssl
{
static bool validateCertificateSha1(const Certificate* certificate, void* data)
{
	uint8_t* hash = static_cast<uint8_t*>(data);
	bool success = false;
	if(hash != nullptr) {
		if(certificate != nullptr) {
			success = certificate->matchFingerprint(hash);
		}
		delete[] hash;
	}

	return success;
}

static bool validatePublicKeySha256(const Certificate* certificate, void* data)
{
	uint8_t* hash = static_cast<uint8_t*>(data);
	bool success = false;
	if(hash != nullptr) {
		if(certificate != nullptr) {
			success = certificate->matchPki(hash);
		}
		delete[] hash;
	}

	return success;
}

bool ValidatorList::validate(const Certificate* certificate)
{
	if(certificate != nullptr && count() == 0) {
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
		if(validator.callback(success ? nullptr : certificate, validator.data)) {
			debug_d("SSL validator: positive match");
			success = true;
		}
		// Callback will have released data so pointer no longer valid
		validator.data = nullptr;
	}

	if(certificate != nullptr && !success) {
		debug_d("SSL validator: NO match");
	}

	return success;
}

bool ValidatorList::add(const uint8_t* fingerprint, FingerprintType type)
{
	Validator::Callback callback = nullptr;
	switch(type) {
	case eSFT_CertSha1:
		callback = validateCertificateSha1;
		break;
	case eSFT_PkSha256:
		callback = validatePublicKeySha256;
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

bool ValidatorList::add(Fingerprints& fingerprints)
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

} // namespace Ssl
