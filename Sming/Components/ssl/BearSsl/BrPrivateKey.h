/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrPrivateKey.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "BrKey.h"

namespace Ssl
{
class BrPrivateKey : public BrKeyTemplate<BrPrivateKey, br_rsa_private_key, br_ec_private_key>
{
public:
	/**
	 * @brief Decode a private key block
	 *
	 * The private key decoder recognises RSA and EC private keys, either in
	 * their raw, DER-encoded format, or wrapped in an unencrypted PKCS#8
	 * archive (again DER-encoded).
	 *
	 */
	bool decode(const uint8_t* buf, size_t len);

private:
	friend BrKeyTemplate<BrPrivateKey, br_rsa_private_key, br_ec_private_key>;
	bool copy(const br_rsa_private_key& rsa);
	bool copy(const br_ec_private_key& ec);
	void freeMem();
};

} // namespace Ssl
