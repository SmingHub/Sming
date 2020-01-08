/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrCertificate.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "BrCertificate.h"
#include "BrContext.h"
#include <Network/Ssl/Session.h>
#include <FlashString/Array.hpp>

namespace Ssl
{
bool BrCertificate::getFingerprint(Fingerprint::Type type, Fingerprint& fingerprint) const
{
	switch(type) {
	case Fingerprint::Type::CertSha1:
		if(fpCertSha1 == nullptr) {
			return false;
		}
		fingerprint.cert.sha1 = *fpCertSha1;
		return true;

	case Fingerprint::Type::CertSha256:
		if(fpCertSha256 == nullptr) {
			return false;
		}
		fingerprint.cert.sha256 = *fpCertSha256;
		return true;

	case Fingerprint::Type::PkiSha256:
		// There is no easy easy way to obtain this.
		return false;

	default:
		return false;
	}
}

String BrCertificate::getName(DN dn, RDN rdn) const
{
#define XX(tag, a, b, c, d) d,
	DEFINE_FSTR_ARRAY_LOCAL(rdnTypes, uint8_t, SSL_X509_RDN_OID_MAP(XX));
#undef XX

	uint8_t type = rdnTypes[unsigned(rdn)];
	if(type == 0) {
		return nullptr;
	}

	switch(dn) {
	case DN::ISSUER:
		return issuer.getRDN(type);
	case DN::SUBJECT:
		return issuer.getRDN(type);
	default:
		return nullptr;
	}
}

} // namespace Ssl
