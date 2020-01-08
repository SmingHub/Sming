/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrKey.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <bearssl.h>

namespace Ssl
{
/**
 * @brief Base class containing common code for private and public keys
 */
class BrKey
{
public:
	enum class Type {
		None = 0,
		RSA = BR_KEYTYPE_RSA,
		EC = BR_KEYTYPE_EC,
	};
};

/**
 * @brief Class template for private/public keys
 * @tparam RsaKey Structure type for an RSA key
 * @tparam EcKey Structure type for an EC key
 */
template <class RsaKey, class EcKey> class BrKeyTemplate : public BrKey
{
public:
	Type getKeyType() const
	{
		return Type(key.type);
	}

	/**
	 * @name Copy operators
	 * @{
	 */
	BrKeyTemplate& operator=(const BrKeyTemplate& rhs)
	{
		if(this != &rhs) {
			this->copy(rhs);
		}
		return *this;
	}

	BrKeyTemplate& operator=(const RsaKey& rsa)
	{
		this->copy(rsa);
		return *this;
	}

	BrKeyTemplate& operator=(const EcKey& ec)
	{
		this->copy(ec);
		return *this;
	}
	/** @} */

	/**
	 * @brief Move operator
	 */
	BrKeyTemplate& operator=(BrKeyTemplate&& rhs)
	{
		move(rhs);
		return *this;
	}

	/**
	 * @brief Access to obtain RSA key pointer
	 *
	 * Returns NULL if not an RSA key
	 */
	operator const RsaKey*() const
	{
		return (key.type == BR_KEYTYPE_RSA) ? &key.rsa : nullptr;
	}

	/**
	 * @brief Access to obtain EC key pointer
	 *
	 * Returns NULL if not an EC key
	 */
	operator const EcKey*() const
	{
		return (key.type == BR_KEYTYPE_EC) ? &key.ec : nullptr;
	}

protected:
	bool copy(const BrKeyTemplate& other)
	{
		switch(other.key.type) {
		case BR_KEYTYPE_RSA:
			return this->copy(other.key.rsa);
		case BR_KEYTYPE_EC:
			return this->copy(other.key.ec);
		default:
			return false;
		}
	}

	void move(BrKeyTemplate& other)
	{
		this->freeMem();
		key = other.key;
		other.key = {};
	}

	// Compatible with br_x509_pkey
	struct {
		uint8_t type;
		union {
			RsaKey rsa;
			EcKey ec;
		};
	} key = {};
};

} // namespace Ssl
