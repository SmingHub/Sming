#include <Network/Ssl/Certificate.h>
#include <FlashString/Vector.hpp>

namespace Ssl
{
#define XX(tag, a, b, c, d) DEFINE_FSTR_LOCAL(rdnStr_##tag, #tag)
SSL_X509_RDN_OID_MAP(XX)
#undef XX

#define XX(tag, a, b, c, d) &rdnStr_##tag,
DEFINE_FSTR_VECTOR_LOCAL(rdnStrings, FSTR::String, SSL_X509_RDN_OID_MAP(XX));
#undef XX

String Certificate::getRdnTypeString(Certificate::RDN rdn)
{
	return rdnStrings[unsigned(rdn)];
}

size_t Certificate::printTo(Print& p) const
{
	size_t n = 0;

	auto printName = [&](DN dn) {
		for(unsigned i = 0; i < unsigned(RDN::MAX); ++i) {
			auto rdn = RDN(i);
			auto s = getName(dn, rdn);
			if(!s) {
				continue;
			}
			n += p.print("  ");
			n += p.print(getRdnTypeString(rdn));
			n += p.print(": ");
			n += p.println(s);
		}
	};

	n += p.println(_F("Subject:"));
	printName(DN::SUBJECT);
	n += p.println(_F("Issuer:"));
	printName(DN::ISSUER);

	return n;
}

} // namespace Ssl
