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

#include <Sodium/SignEd25519.h>
#include <FlashString/Array.hpp>

namespace OtaUpgrade
{
DECLARE_FSTR_ARRAY(SignatureVerificationKey, uint8_t)

/**
 * @brief Signature verifier for `BasicStream`.
 *
 * This is a simple C++ wrapper for Libsodium's 
 * <a href="https://download.libsodium.org/doc/public-key_cryptography/public-key_signatures">`crypto_sign_...`</a> API.
 */
class SignatureVerifier : public Sodium::SignEd25519
{
public:
	using VerificationData = Signature;

	/** Check if \a signature matches the data previously fed into #update() calls.
	 * @return `true` if \a signature matches data, `false` otherwise.
	 */
	bool verify(const Signature& signature)
	{
		PrivateKey key;
		assert(SignatureVerificationKey.length() == key.size());
		SignatureVerificationKey.read(0, key.data(), key.size());
		return Sodium::SignEd25519::verify(signature, key);
	}
};

} // namespace OtaUpgrade
