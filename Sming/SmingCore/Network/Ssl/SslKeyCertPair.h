/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_SSLKEYCERTPAIR_H_
#define SMINGCORE_NETWORK_SSLKEYCERTPAIR_H_

#include "ssl/ssl.h"

/** @brief Structure to manage an SSL key certificate with optional password
 *  @note Do not set member variables directly, use provided methods
 */
struct SslKeyCertPair {
	uint8_t* key = nullptr;
	unsigned keyLength = 0;
	char* keyPassword = nullptr;
	uint8_t* certificate = nullptr;
	unsigned certificateLength = 0;

	~SslKeyCertPair()
	{
		free();
	}

	bool isValid()
	{
		return keyLength != 0 && certificateLength != 0;
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
			key = new uint8_t[newKeyLength];
			if(key == nullptr) {
				return false;
			}
			memcpy(key, newKey, newKeyLength);
			keyLength = newKeyLength;
		}

		if(newCertificateLength != 0 && newCertificate != nullptr) {
			certificate = new uint8_t[newCertificateLength];
			if(certificate == nullptr) {
				return false;
			}
			memcpy(certificate, newCertificate, newCertificateLength);
			certificateLength = newCertificateLength;
		}

		unsigned passwordLength = (newKeyPassword == nullptr) ? 0 : strlen(newKeyPassword);
		if(passwordLength != 0) {
			keyPassword = new char[passwordLength + 1];
			if(keyPassword == nullptr) {
				return false;
			}
			memcpy(keyPassword, newKeyPassword, passwordLength);
			keyPassword[passwordLength] = '\0';
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
		return assign(keyCert.key, keyCert.keyLength, keyCert.certificate, keyCert.certificateLength,
					  keyCert.keyPassword);
	}

	SslKeyCertPair& operator=(const SslKeyCertPair& keyCert)
	{
		assign(keyCert);
		return *this;
	}

	void free()
	{
		delete[] key;
		key = nullptr;

		delete[] certificate;
		certificate = nullptr;

		delete[] keyPassword;
		keyPassword = nullptr;

		keyLength = 0;
		certificateLength = 0;
	}
};

#endif /* SMINGCORE_NETWORK_SSLKEYCERTPAIR_H_ */
