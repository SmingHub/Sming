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

/**
 * @brief Signature verifier for `BasicOtaUpgradeStream`.
 *
 * This is a simple C++ wrapper for Libsodium's 
 * <a href="https://download.libsodium.org/doc/public-key_cryptography/public-key_signatures">`crypto_sign_...`</a> API.
 */
class OtaSignatureVerifier
{
	crypto_sign_state state;

public:
	static const size_t NumBytes = crypto_sign_BYTES; ///< Size of signature in bytes.

	OtaSignatureVerifier()
	{
		crypto_sign_init(&state);
	}

	/** Continue incremental calculation for given chunk of data.
	 */
	void update(const void* data, size_t size)
	{
		crypto_sign_update(&state, static_cast<const unsigned char*>(data), size);
	}

	/** Check if \a signature matches the data previously fed into #update() calls.
	 * @return `true` if \a signature matches data, `false` otherwise.
	 */
	bool verify(const uint8_t (&signature)[crypto_sign_BYTES])
	{
		assert(OTAUpgrade_SignatureVerificationKey.length() == crypto_sign_PUBLICKEYBYTES);
		LOAD_FSTR_ARRAY(verificationKey, OTAUpgrade_SignatureVerificationKey);
		return (crypto_sign_final_verify(&state, signature, verificationKey) == 0);
	}
};
