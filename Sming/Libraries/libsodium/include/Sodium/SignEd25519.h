/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SignEd25519.h
 *
 ****/

#pragma once

#include <Crypto/HashContext.h>

namespace Sodium
{
class SignEd25519 : public Crypto::Sha512
{
public:
	using Signature = Crypto::Sha512::Hash;
	using PrivateKey = Crypto::ByteArray<32>;

	bool verify(const Signature& sig, const PrivateKey& key);
};

} // namespace Sodium
