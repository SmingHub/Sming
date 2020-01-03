/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxCertificate.cpp
 *
 ****/

#include "AxCertificate.h"

namespace Ssl
{
String AxCertificate::getName(DN dn, RDN rdn) const
{
	if(ssl == nullptr || ssl->x509_ctx == nullptr) {
		return nullptr;
	}

	int dnType;
	switch(rdn) {
	case RDN::COMMON_NAME:
		dnType = X509_COMMON_NAME;
		break;
	case RDN::ORGANIZATION_NAME:
		dnType = X509_ORGANIZATION;
		break;
	case RDN::ORGANIZATIONAL_UNIT_NAME:
		dnType = X509_ORGANIZATIONAL_UNIT;
		break;
	case RDN::LOCALITY_NAME:
		dnType = X509_LOCATION;
		break;
	case RDN::COUNTRY_NAME:
		dnType = X509_COUNTRY;
		break;
	case RDN::STATE_OR_PROVINCE_NAME:
		dnType = X509_STATE;
		break;
	default:
		return nullptr;
	}

	switch(dn) {
	case DN::ISSUER:
		return ssl->x509_ctx->ca_cert_dn[dnType];

	case DN::SUBJECT:
		return ssl->x509_ctx->cert_dn[dnType];
	default:
		return nullptr;
	}
}

} // namespace Ssl
