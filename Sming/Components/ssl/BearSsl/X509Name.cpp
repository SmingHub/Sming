/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * X509Name.cpp
 *
 ****/

#include <SslDebug.h>
#include "X509Name.h"
#include "Asn1Parser.h"

namespace Ssl
{
void X509Name::append(const void* buf, size_t len)
{
	br_sha256_update(&sha256, buf, len);
	dn.concat(static_cast<const char*>(buf), len);
}

String X509Name::getRDN(uint8_t type) const
{
	Asn1Parser parser(reinterpret_cast<const uint8_t*>(dn.c_str()), dn.length());

	if(parser.getNextObject(ASN1_SEQUENCE) < 0) {
		return nullptr;
	}

	unsigned len;
	while((len = parser.getNextObject(ASN1_SET)) != 0) {
		auto nextOffset = parser.getOffset() + len;

		if(parser.getNextObject(ASN1_SEQUENCE) == 0) {
			break;
		}

		int dn_type = parser.getObjectId();
		if(dn_type < 0) {
			break;
		}

		if(dn_type == type) {
			return parser.getString();
		}

		parser.setOffset(nextOffset);
	}

	return nullptr;
}

} // namespace Ssl
