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
 *
 *  Use within application's SSL initialisation callback to simplify setting standard
 *  fingerprints for validation. For example:
 *
 *		void mySslInitCallback(Ssl::Session& session, HttpRequest& request)
 *		{
 *			static const uint8_t sha1Fingerprint[] PROGMEM = {
 *				0x15, 0x9A, 0x76, 0xC5, 0xAE, 0xF4, 0x90, 0x15, 0x79, 0xE6,
 *				0xA4, 0x99, 0x96, 0xC1, 0xD6, 0xA1, 0xD9, 0x3B, 0x07, 0x43
 *			};
 *			Ssl::Fingerprints fingerprints;
 *			fingerprints.setSha1_P(sha1Fingerprint, sizeof(sha1Fingerprint));
 *			session.validators.add(fingerprints);
 *		}
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
	Fingerprints& operator=(Fingerprints& source);

	/** @brief Make copy of values from source */
	Fingerprints& operator=(const Fingerprints& source);

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

/**
 * @deprecated Use Ssl::FingerprintType instead
 */
typedef Ssl::FingerprintType SslFingerprintType SMING_DEPRECATED;

/**
 * @deprecated Use Ssl::FingerprintType instead
 */
typedef Ssl::FingerprintType SSLFingerprintType SMING_DEPRECATED;

/**
 * @deprecated Use Ssl::Fingerprints instead
 */
typedef Ssl::Fingerprints SslFingerprints SMING_DEPRECATED;

/**
 * @deprecated Use Ssl::Fingerprints instead
 */
typedef Ssl::Fingerprints SSLFingerprints SMING_DEPRECATED;
