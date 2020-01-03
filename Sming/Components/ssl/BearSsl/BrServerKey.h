/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrServerKey.h
 *
 ****/

#pragma once

#include <bearssl.h>

namespace Ssl
{
/**
 * @brief Decodes a private key blob into BR compatible format
 */
class BrServerKey
{
public:
	enum class Type {
		None = 0,
		RSA = BR_KEYTYPE_RSA,
		EC = BR_KEYTYPE_EC,
	};

	~BrServerKey()
	{
		freeMem();
	}

	int decode(const uint8_t* buf, size_t len);

	void freeMem();

	Type getKeyType() const
	{
		return keyType;
	}

	operator const br_rsa_private_key*() const
	{
		return (keyType == Type::RSA) ? &key.rsa : nullptr;
	}

	operator const br_ec_private_key*() const
	{
		return (keyType == Type::EC) ? &key.ec : nullptr;
	}

private:
	Type keyType = Type::None;
	union {
		br_rsa_private_key rsa;
		br_ec_private_key ec;
	} key = {};
};

} // namespace Ssl
