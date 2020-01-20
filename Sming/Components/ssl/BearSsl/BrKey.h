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
template <class KeyClass, class RsaKey, class EcKey> class BrKeyTemplate : public BrKey
{
public:
	~BrKeyTemplate()
	{
		static_cast<KeyClass*>(this)->freeMem();
	}

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
			copy(rhs.key);
		}
		return *this;
	}

	BrKeyTemplate& operator=(const RsaKey& rsa)
	{
		static_cast<KeyClass*>(this)->copy(rsa);
		return *this;
	}

	BrKeyTemplate& operator=(const EcKey& ec)
	{
		static_cast<KeyClass*>(this)->copy(ec);
		return *this;
	}
	/** @} */

	/**
	 * @brief Move operator
	 */
	BrKeyTemplate& operator=(BrKeyTemplate&& rhs)
	{
		move(rhs.key);
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

	/**
	 * @brief Test for validity
	 */
	operator bool() const
	{
		return key.type != 0;
	}

protected:
	// Compatible with br_x509_pkey
	struct Key {
		uint8_t type;
		union {
			RsaKey rsa;
			EcKey ec;
		};
	};

	bool copy(const Key& other)
	{
		switch(other.type) {
		case BR_KEYTYPE_RSA:
			return static_cast<KeyClass*>(this)->copy(other.rsa);
		case BR_KEYTYPE_EC:
			return static_cast<KeyClass*>(this)->copy(other.ec);
		default:
			static_cast<KeyClass*>(this)->freeMem();
			return other.type == 0;
		}
	}

	void move(Key& other)
	{
		static_cast<KeyClass*>(this)->freeMem();
		key = other;
		other = {};
	}

	Key key{};
};

} // namespace Ssl
