/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OtaSignatureVerifier.h
 *
 ****/

#pragma once

#include <sodium/crypto_sign.h>
#include <FlashString/Array.hpp>

DECLARE_FSTR_ARRAY(OTAUpgrade_SignatureVerificationKey, uint8_t)

class OtaSignatureVerifier
{
	crypto_sign_state state;

public:
	static const size_t NumBytes = crypto_sign_BYTES;

	OtaSignatureVerifier()
	{
		crypto_sign_init(&state);
	}

	void update(const void* data, size_t size)
	{
		crypto_sign_update(&state, static_cast<const unsigned char*>(data), size);
	}

	bool verify(const uint8_t (&signature)[crypto_sign_BYTES])
	{
		assert(OTAUpgrade_SignatureVerificationKey.length() == crypto_sign_PUBLICKEYBYTES);
		LOAD_FSTR_ARRAY(verificationKey, OTAUpgrade_SignatureVerificationKey);
		return (crypto_sign_final_verify(&state, signature, verificationKey) == 0);
	}
};
