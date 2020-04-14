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

#include <Crypto/Sha1.h>
#include <Crypto/Sha2.h>
#include <BitManipulations.h>

namespace Ssl
{
/**
 * @brief Various types of fingerprint
 *
 * Applications should use the appropriate type to define a fingerprint, for example:
 *
 * 		static const Fingerprint::Cert::Sha1 fingerprint PROGMEM = { ... };
 *
 */
union Fingerprint {
	/**
	 * @brief SSL Certificate fingerprint type
	 */
	enum class Type {
		CertSha1,   ///< SHA1 Fingerprint of entire certificate
		CertSha256, ///< SHA256 Fingerprint of entire certificate
		PkiSha256,  ///< SHA256 Fingerprint of Public Key Information
	};
	/**
	 * @brief Maintains a set of fingerprint types
	 */
	class Types
	{
	public:
		void add(Type type)
		{
			bitSet(mask, type);
		}

		void remove(Type type)
		{
			bitClear(mask, type);
		}

		bool contains(Type type) const
		{
			return bitRead(mask, type);
		}

	private:
		uint32_t mask = 0;
	};

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
			static constexpr Type type = Type::CertSha1;
			Crypto::Sha1::Hash hash;
		};
		Sha1 sha1;

		/** @brief Fingerprint based on the SHA256 value of the certificate
		 *
		 * 	Typically displayed in browser certificate information
		 */
		struct Sha256 {
			static constexpr Type type = Type::CertSha256;
			Crypto::Sha256::Hash hash;
		};
		Sha256 sha256;
	};
	Cert cert;

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
			static constexpr Type type = Type::PkiSha256;
			Crypto::Sha256::Hash hash;
		};
		Sha256 sha256;
	};
	Pki pki;
};

} // namespace Ssl
