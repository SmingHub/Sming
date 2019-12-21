#include <Network/Ssl/Connection.h>
#include <Print.h>

namespace Ssl
{
String cipherSuiteName(CipherSuite id)
{
	switch(id) {
#define XX(n1, n2, tag)                                                                                                \
	case CipherSuite::tag:                                                                                             \
		return F(#tag);
		SSL_CIPHER_SUITE_MAP(XX)
#undef XX
	default: {
		char buf[32];
		auto len = m_snprintf(buf, sizeof(buf), _F("{ 0x%02X, 0x%02X }"), unsigned(id) >> 8, unsigned(id) & 0xFF);
		return String(buf, len);
	}
	}
}

size_t Connection::printTo(Print& p) const
{
	size_t n = 0;
	n += p.println(_F("SSL Connection Information:"));
	auto& cert = getCertificate();
	if(cert.isValid()) {
		n += p.print(_F("  Certificate:  "));
		n += p.println(cert.getName(Certificate::Name::CERT_COMMON_NAME));
	}
	n += p.print(_F("  Cipher:       "));
	n += p.println(cipherSuiteName(getCipherSuite()));
	n += p.print(_F("  Session ID:   "));
	n += p.println(getSessionId());
	return n;
}

} // namespace Ssl
