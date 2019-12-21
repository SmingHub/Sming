/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Fingerprints.h
 *
 ****/

#pragma once

#include "Crypto.h"
#include <sming_attr.h>

namespace Ssl
{
/**
 * @brief SSL Certificate fingerprint type
 * @ingroup ssl
 */
enum FingerprintType {
	/** @brief Fingerprint based on the SHA1 value of the certificate
	 * 	@note The SHA1 hash of the entire certificate. This changes on each certificate renewal so needs
	 * 	to be updated every time the remote server updates its certificate.
	 * 	Advantages: Takes less time to verify than SHA256
	 * 	Disadvantages: Likely to change periodically
	 */
	eSFT_CertSha1 = 0,

	/**	@brief Fingerprint based on the SHA256 value of the Public Key Subject in the certificate
	 * 	@note For HTTP public key pinning (RFC7469), the SHA-256 hash of the Subject Public Key Info
	 * 	(which usually only changes when the public key changes) is used.
	 * 	Advantages: Doesn't change frequently
	 * 	Disadvantages: Takes more time (in ms) to verify.
	 */
	eSFT_PkSha256,
};

/** @brief Contains SSL fingerprint data
 *  @ingroup ssl
 *  @note Lifetime as follows:
 *  	- Constructed by application, using appropriate setXXX method;
 *  	- Passed into HttpRequest by application, using pinCertificate method - request is then queued;
 *  	- Passed into HttpClientConnection (TcpClient descendant) by HttpClient, using pinCertificate method
 *  	- When certificate validated, memory is released
 *
 */
struct Fingerprints {
	const uint8_t* certSha1 = nullptr; ///< certificate SHA1 fingerprint
	const uint8_t* pkSha256 = nullptr; ///< public key SHA256 fingerprint

	~Fingerprints()
	{
		free();
	}

	void free();

	/** @brief Set the SHA1 fingerprint
	 *  @param cert data to copy
	 *  @param length for checking
	 *  @retval bool false on length check or allocation failure
	 */
	bool setSha1(const uint8_t* cert, unsigned length)
	{
		return setValue(certSha1, SHA1_SIZE, cert, length);
	}

	/** @brief Make copy of SHA1 certificate from data stored in flash */
	bool setSha1_P(const uint8_t* cert, unsigned length)
	{
		return setValue(certSha1, SHA1_SIZE, cert, length);
	}

	/** @brief Set the SHA256 fingerprint
	 *  @param cert data to copy
	 *  @param length for checking
	 *  @retval bool false on length check or allocation failure
	 */
	bool setSha256(const uint8_t* cert, unsigned length)
	{
		return setValue(pkSha256, SHA256_SIZE, cert, length);
	}

	/** @brief Make copy of SHA256 certificate from data stored in flash */
	bool setSha256_P(const uint8_t* cert, unsigned length)
	{
		return setValue(pkSha256, SHA256_SIZE, cert, length);
	}

	/** @brief Moves values out of source */
	Ssl::Fingerprints& operator=(Ssl::Fingerprints& source);

	/** @brief Make copy of values from source */
	Ssl::Fingerprints& operator=(const Ssl::Fingerprints& source);

private:
	/** @brief Internal method to set a fingerprint
	 *  @param value Reference to fingerprint value in this structure
	 *  @param requiredLength Expected length for value
	 *  @param newValue
	 *  @param newLength
	 *  @retval bool true on success, false on invalid length or memory allocation failure
	 */
	bool setValue(const uint8_t*& value, unsigned requiredLength, const uint8_t* newValue, unsigned newLength);
};

} // namespace Ssl

typedef Ssl::FingerprintType SslFingerprintType;
typedef Ssl::Fingerprints SslFingerprints;

typedef Ssl::FingerprintType SSLFingerprintType SMING_DEPRECATED; ///< @deprecated Use Ssl::FingerprintType instead
typedef Ssl::Fingerprints SSLFingerprints SMING_DEPRECATED;		  ///< @deprecated Use Ssl::Fingerprints instead
