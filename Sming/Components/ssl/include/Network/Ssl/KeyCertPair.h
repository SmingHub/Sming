/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * KeyCertPair.h
 *
 ****/

#pragma once

#include "WString.h"

namespace Ssl
{
/**
 * @brief Class to manage an SSL key certificate with optional password
 * @ingroup ssl
 */
class KeyCertPair
{
public:
	bool isValid() const
	{
		return key && certificate;
	}

	/** @brief Create certificate using provided values
	 *  @param newKey
	 *  @param newKeyLength
	 *  @param newCertificate
	 *  @param newCertificateLength
	 *  @param newKeyPassword
	 *  @retval bool false on memory allocation failure
	 *  @note We take a new copy of the certificate
	 *  @{
	 */
	bool assign(const uint8_t* newKey, unsigned newKeyLength, const uint8_t* newCertificate,
				unsigned newCertificateLength, const char* newKeyPassword = nullptr);

	bool assign(String newKey, String newCertificate, const char* newKeyPassword = nullptr)
	{
		key = newKey;
		certificate = newCertificate;
		return key && certificate && setPassword(newKeyPassword);
	}
	/** @} */

	/** @brief Assign another certificate to this structure
	 *  @param keyCert
	 *  @retval bool false on memory allocation failure
	 *  @note We take a new copy of the certificate
	 */
	bool assign(const KeyCertPair& keyCert)
	{
		*this = keyCert;
		return (key == keyCert.key) && (keyPassword == keyCert.keyPassword) && (certificate == keyCert.certificate);
	}

	void free()
	{
		key = nullptr;
		keyPassword = nullptr;
		certificate = nullptr;
	}

	const uint8_t* getKey() const
	{
		return reinterpret_cast<const uint8_t*>(key.c_str());
	}

	unsigned getKeyLength() const
	{
		return key.length();
	}

	const char* getKeyPassword() const
	{
		return keyPassword.c_str();
	}

	const uint8_t* getCertificate() const
	{
		return reinterpret_cast<const uint8_t*>(certificate.c_str());
	}

	unsigned getCertificateLength() const
	{
		return certificate.length();
	}

private:
	bool setPassword(const char* newKeyPassword);

private:
	String key;
	String keyPassword;
	String certificate;
};

} // namespace Ssl

typedef Ssl::KeyCertPair SslKeyCertPair SMING_DEPRECATED; ///< @deprecated Use SslKeyCertPair instead

typedef Ssl::KeyCertPair SSLKeyCertPair SMING_DEPRECATED; ///< @deprecated Use SslKeyCertPair instead
