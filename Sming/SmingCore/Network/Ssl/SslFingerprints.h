/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "ssl/ssl.h"

enum SslFingerprintType {
	eSFT_CertSha1 = 0, // << Fingerprint based on the SHA1 value of the certificate.
					   //     Every time a certificate is renewed this value will change.
	eSFT_PkSha256,	 // << Fingerprint based on the SHA256 value of the public key subject in the certificate.
					   //    Only when the private key used to generate the certificate is used then that fingerprint
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

	void setSha1(const uint8_t* cert)
	{
		if(cert == nullptr) {
			delete[] certSha1;
			certSha1 = nullptr;
		} else {
			if(certSha1 == nullptr) {
				certSha1 = new uint8_t[SHA1_SIZE];
			}
			memcpy(certSha1, cert, SHA1_SIZE);
		}
	}

	void setSha256(const uint8_t* cert)
	{
		if(cert == nullptr) {
			delete[] pkSha256;
			pkSha256 = nullptr;
		} else {
			if(pkSha256 == nullptr) {
				pkSha256 = new uint8_t[SHA256_SIZE];
			}
			memcpy(pkSha256, cert, SHA256_SIZE);
		}
	}

	/** @brief Make copy of SHA1 certificate from data stored in flash
	 *  @param cert
	 */
	void setSha1_P(const uint8_t* cert)
	{
		// Word-aligned and sized buffers don't need special handling
		setSha1(cert);
	}

	/** @brief Make copy of SHA256 certificate from data stored in flash
	 *  @param cert
	 */
	void setSha256_P(const uint8_t* cert)
	{
		// Word-aligned and sized buffers don't need special handling
		setSha256(cert);
	}

	SslFingerprints& operator=(SslFingerprints& source)
	{
		delete[] certSha1;
		certSha1 = source.certSha1;
		source.certSha1 = nullptr;

		delete[] pkSha256;
		pkSha256 = source.pkSha256;
		source.pkSha256 = nullptr;

		return *this;
	}

	SslFingerprints& operator=(const SslFingerprints& source)
	{
		setSha1(source.certSha1);
		setSha256(source.pkSha256);

		return *this;
	}
};
