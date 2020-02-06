/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CipherSuite.cpp
 *
 ****/

#include <Network/Ssl/CipherSuite.h>
#include <FlashString/Map.hpp>

namespace Ssl
{
namespace CipherSuites
{
// clang-format off
DEFINE_CIPHER_SUITES(
		basic,
		CipherSuite::RSA_WITH_AES_128_CBC_SHA256,
		CipherSuite::RSA_WITH_AES_256_CBC_SHA256,
		CipherSuite::RSA_WITH_AES_128_CBC_SHA,
		CipherSuite::RSA_WITH_AES_256_CBC_SHA,
	)
// clang-format on
} // namespace CipherSuites
#define XX(tag, code) DEFINE_FSTR_LOCAL(cipherSuite_##tag, #tag)

SSL_CIPHER_SUITE_MAP(XX)
#undef XX

#define XX(tag, code) {CipherSuite::tag, &cipherSuite_##tag},
DEFINE_FSTR_MAP_LOCAL(cipherSuiteNames, CipherSuite, FSTR::String, SSL_CIPHER_SUITE_MAP(XX));
#undef XX

String toString(CipherSuite id)
{
	auto entry = cipherSuiteNames[id];
	if(entry) {
		return String(entry);
	}

	char buf[32];
	auto len = m_snprintf(buf, sizeof(buf), _F("{ 0x%04X }"), unsigned(id));
	return String(buf, len);
}

} // namespace Ssl
