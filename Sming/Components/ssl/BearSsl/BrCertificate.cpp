/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrCertificate.cpp
 *
 ****/

#include "BrCertificate.h"
#include <FlashString/Array.hpp>

namespace Ssl
{
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
		return context->getIssuer().getRDN(type);
	case DN::SUBJECT:
		return context->getSubject().getRDN(type);
	default:
		return nullptr;
	}
}

} // namespace Ssl
