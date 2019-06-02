/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslKeyCertPair.h
 *
 ****/

#pragma once

#include "ssl/ssl.h"
#include "WString.h"

/** @brief Class to manage an SSL key certificate with optional password
 */
class SslKeyCertPair
{
public:
	bool isValid()
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
	 */
	bool assign(const uint8_t* newKey, unsigned newKeyLength, const uint8_t* newCertificate,
				unsigned newCertificateLength, const char* newKeyPassword = nullptr)
	{
		free();

		if(newKeyLength != 0 && newKey != nullptr) {
			if(!key.setLength(newKeyLength)) {
				return false;
			}
			memcpy(key.begin(), newKey, newKeyLength);
		}

		if(newCertificateLength != 0 && newCertificate != nullptr) {
			if(!certificate.setLength(newCertificateLength)) {
				return false;
			}
			memcpy(certificate.begin(), newCertificate, newCertificateLength);
		}

		unsigned passwordLength = (newKeyPassword == nullptr) ? 0 : strlen(newKeyPassword);
		if(passwordLength != 0) {
			keyPassword.setString(newKeyPassword, passwordLength);
			if(!keyPassword) {
				return false;
			}
		}

		return true;
	}

	/** @brief Assign another certificate to this structure
	 *  @param keyCert
	 *  @retval bool false on memory allocation failure
	 *  @note We take a new copy of the certificate
	 */
	bool assign(const SslKeyCertPair& keyCert)
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

	const uint8_t* getKey()
	{
		return reinterpret_cast<const uint8_t*>(key.c_str());
	}

	unsigned getKeyLength()
	{
		return key.length();
	}

	const char* getKeyPassword()
	{
		return keyPassword.c_str();
	}

	const uint8_t* getCertificate()
	{
		return reinterpret_cast<const uint8_t*>(certificate.c_str());
	}

	unsigned getCertificateLength()
	{
		return certificate.length();
	}

private:
	String key;
	String keyPassword;
	String certificate;
};
