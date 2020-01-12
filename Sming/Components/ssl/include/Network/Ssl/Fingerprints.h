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

#include <Crypto.h>
#include <BitManipulations.h>

namespace Ssl
{
/**
 * @brief Various types of fingerprint
 *
 * This structure provides a convenient way to define unique types for each fingerprint.
 *
 * It's not intended to actually be used as a union, that just implies we're picking one of them,
 * not all of them.
 *
 * For example:
 *
 * 		Fingerprint::Cert::Sha1 fingerprint = { ... };
 */
union Fingerprint {
	/**
	 * @brief Fingerprints for the entire Certificate
	 */
	union Cert {
		/** @brief Fingerprint based on the SHA1 value of the certificate
		 *
		 * 	The SHA1 hash of the entire certificate. This changes on each certificate renewal so needs
		 * 	to be updated every time the remote server updates its certificate.
		 *
		 * 	Advantages: Takes less time to verify than SHA256
		 *
		 * 	Disadvantages: Likely to change periodically
		 */
		struct Sha1 {
			Crypto::Sha1::Hash hash;
		};

		/** @brief Fingerprint based on the SHA256 value of the certificate
		 *
		 * 	Typically displayed in browser certificate information
		 */
		struct Sha256 {
			Crypto::Sha256::Hash hash;
		};

	}; // namespace Cert

	/**
	 * @Fingerprints for the Public Key only
	 */
	union Pki {
		/**	@brief Fingerprint based on the SHA256 value of the Public Key Subject in the certificate
		 *
		 * 	For HTTP public key pinning (RFC7469), the SHA-256 hash of the Subject Public Key Info
		 * 	(which usually only changes when the public key changes) is used.
		 *
		 * 	Advantages: Doesn't change frequently
		 *
		 * 	Disadvantages: Takes more time (in ms) to verify.
		 */
		struct Sha256 {
			Crypto::Sha256::Hash hash;
		};

	}; // namespace Pki

}; // namespace Fingerprint

} // namespace Ssl
