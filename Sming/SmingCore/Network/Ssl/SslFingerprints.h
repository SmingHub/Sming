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

typedef struct {
	uint8_t* certSha1 = nullptr; // << certificate SHA1 fingerprint
	uint8_t* pkSha256 = nullptr; // << public key SHA256 fingerprint

	void free()
	{
		delete certSha1;
		certSha1 = nullptr;
		delete pkSha256;
		pkSha256 = nullptr;
	}
} SslFingerprints;
