/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrPublicKey.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "BrKey.h"

namespace Ssl
{
class BrPublicKey : public BrKeyTemplate<BrPublicKey, br_rsa_public_key, br_ec_public_key>
{
public:
	operator const br_x509_pkey*() const
	{
		return reinterpret_cast<const br_x509_pkey*>(&key);
	}

	BrPublicKey& operator=(const br_x509_pkey* rhs)
	{
		if(rhs == nullptr) {
			freeMem();
		} else {
			BrKeyTemplate::copy(*reinterpret_cast<const Key*>(rhs));
		}
		return *this;
	}

	/**
	 * @brief Decode a public key block
	 *
	 * The public key decoder recognises RSA and EC private keys, either in
	 * their raw, DER-encoded format, or wrapped in an unencrypted PKCS#8
	 * archive (again DER-encoded).
	 *
	 */
	bool decode(const uint8_t* buf, size_t len);

private:
	friend BrKeyTemplate<BrPublicKey, br_rsa_public_key, br_ec_public_key>;
	bool copy(const br_rsa_public_key& rsa);
	bool copy(const br_ec_public_key& ec);
	void freeMem();
};

} // namespace Ssl
