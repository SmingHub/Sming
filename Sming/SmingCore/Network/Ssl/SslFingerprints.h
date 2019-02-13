/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_SSLFINGERPRINTS_H_
#define SMINGCORE_NETWORK_SSLFINGERPRINTS_H_

#include "ssl/ssl.h"

/**
 * @brief SSL Certificate fingerprint type
 */
enum SslFingerprintType {
	/* The SHA1 hash of the entire certificate. This changes on each certificate renewal so needs
	 * to be updated every time the remote server updates its certificate.
	 * Advantages: Takes less time to verify than SHA256
	 * Disadvantages: Likely to change periodically
	 */
	eSFT_CertSha1 = 0, ///< Fingerprint based on the SHA1 value of the certificate

	/* For HTTP public key pinning (RFC7469), the SHA-256 hash of the Subject Public Key Info
	 * (which usually only changes when the public key changes) is used.
	 * Advantages: Doesn't change frequently
	 * Disadvantages: Takes more time (in ms) to verify.
	 */
	eSFT_PkSha256, // << Fingerprint based on the SHA256 value of the Public Key Subject in the certificate
};

/** @brief Contains SSL fingerprint data
 *  @note Lifetime as follows:
 *  	- Constructed by application, using appropriate setXXX method;
 *  	- Passed into HttpRequest by application, using pinCertificate method - request is then queued;
 *  	- Passed into HttpConnection (TcpClient descendant) by HttpClient, using pinCertificate method
 *  	- When certificate validated, memory is released
 *
 */
struct SslFingerprints {
	uint8_t* certSha1 = nullptr; // << certificate SHA1 fingerprint
	uint8_t* pkSha256 = nullptr; // << public key SHA256 fingerprint

	~SslFingerprints()
	{
		free();
	}

	void free()
	{
		delete[] certSha1;
		certSha1 = nullptr;
		delete[] pkSha256;
		pkSha256 = nullptr;
	}

	bool setSha1(const uint8_t* cert, unsigned length)
	{
		return setValue(certSha1, SHA1_SIZE, cert, length);
	}

	bool setSha256(const uint8_t* cert, unsigned length)
	{
		return setValue(pkSha256, SHA256_SIZE, cert, length);
	}

	/** @brief Make copy of SHA1 certificate from data stored in flash
	 *  @param cert
	 */
	bool setSha1_P(const uint8_t* cert, unsigned length)
	{
		// Word-aligned and sized buffers don't need special handling
		return setSha1(cert, length);
	}

	/** @brief Make copy of SHA256 certificate from data stored in flash
	 *  @param cert
	 */
	bool setSha256_P(const uint8_t* cert, unsigned length)
	{
		// Word-aligned and sized buffers don't need special handling
		return setSha256(cert, length);
	}

	/** @brief Moves values out of source */
	SslFingerprints& operator=(SslFingerprints& source)
	{
		if(this != &source) {
			delete[] certSha1;
			certSha1 = source.certSha1;
			source.certSha1 = nullptr;

			delete[] pkSha256;
			pkSha256 = source.pkSha256;
			source.pkSha256 = nullptr;
		}

		return *this;
	}

	/** @brief Make copy of values from source */
	SslFingerprints& operator=(const SslFingerprints& source)
	{
		if(this != &source) {
			setSha1(source.certSha1, SHA1_SIZE);
			setSha256(source.pkSha256, SHA256_SIZE);
		}

		return *this;
	}

private:
	/** @brief Internal method to set a fingerprint
	 *  @param value Reference to fingerprint value in this structure
	 *  @param length Required length for value
	 *  @param newValue
	 *  @param newLength
	 *  @retval bool true on success, false on invalid length or memory allocation failure
	 */
	bool setValue(uint8_t*& value, unsigned length, const uint8_t* newValue, unsigned newLength)
	{
		if(newValue == nullptr || newLength == 0) {
			delete[] value;
			value = nullptr;
			return true;
		} else if(newLength != length) {
			debug_e("ERROR! Invalid fingerprint length");
			return false;
		} else {
			if(value == nullptr) {
				value = new uint8_t[length];
				if(value == nullptr) {
					return false;
				}
			}
			memcpy(value, newValue, length);
			return true;
		}
	}
};

#endif // SMINGCORE_NETWORK_SSLFINGERPRINTS_H_
