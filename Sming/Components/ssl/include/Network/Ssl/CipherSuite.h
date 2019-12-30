/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CipherSuite.h
 *
 ****/

#pragma once

#include <WString.h>

namespace Ssl
{
/**
 * @ingroup ssl
 * @{
 */

/*
 * Cipher suites
 *
 *	TLS v1.2	https://tools.ietf.org/html/rfc5246#appendix-A.5
 * 	TLS v1.3	https://tools.ietf.org/html/rfc8446#appendix-B.4
 *
 * Courtesy of Bear SSL. Defined here so they're not tied into any specific implementation.
 *
 */
#define SSL_CIPHER_SUITE_MAP(XX)                                                                                       \
	/* From RFC 5246 */                                                                                                \
	XX(NULL_WITH_NULL_NULL, 0x0000)                                                                                    \
	XX(RSA_WITH_NULL_MD5, 0x0001)                                                                                      \
	XX(RSA_WITH_NULL_SHA, 0x0002)                                                                                      \
	XX(RSA_WITH_NULL_SHA256, 0x003B)                                                                                   \
	XX(RSA_WITH_RC4_128_MD5, 0x0004)                                                                                   \
	XX(RSA_WITH_RC4_128_SHA, 0x0005)                                                                                   \
	XX(RSA_WITH_3DES_EDE_CBC_SHA, 0x000A)                                                                              \
	XX(RSA_WITH_AES_128_CBC_SHA, 0x002F)                                                                               \
	XX(RSA_WITH_AES_256_CBC_SHA, 0x0035)                                                                               \
	XX(RSA_WITH_AES_128_CBC_SHA256, 0x003C)                                                                            \
	XX(RSA_WITH_AES_256_CBC_SHA256, 0x003D)                                                                            \
	XX(DH_DSS_WITH_3DES_EDE_CBC_SHA, 0x000D)                                                                           \
	XX(DH_RSA_WITH_3DES_EDE_CBC_SHA, 0x0010)                                                                           \
	XX(DHE_DSS_WITH_3DES_EDE_CBC_SHA, 0x0013)                                                                          \
	XX(DHE_RSA_WITH_3DES_EDE_CBC_SHA, 0x0016)                                                                          \
	XX(DH_DSS_WITH_AES_128_CBC_SHA, 0x0030)                                                                            \
	XX(DH_RSA_WITH_AES_128_CBC_SHA, 0x0031)                                                                            \
	XX(DHE_DSS_WITH_AES_128_CBC_SHA, 0x0032)                                                                           \
	XX(DHE_RSA_WITH_AES_128_CBC_SHA, 0x0033)                                                                           \
	XX(DH_DSS_WITH_AES_256_CBC_SHA, 0x0036)                                                                            \
	XX(DH_RSA_WITH_AES_256_CBC_SHA, 0x0037)                                                                            \
	XX(DHE_DSS_WITH_AES_256_CBC_SHA, 0x0038)                                                                           \
	XX(DHE_RSA_WITH_AES_256_CBC_SHA, 0x0039)                                                                           \
	XX(DH_DSS_WITH_AES_128_CBC_SHA256, 0x003E)                                                                         \
	XX(DH_RSA_WITH_AES_128_CBC_SHA256, 0x003F)                                                                         \
	XX(DHE_DSS_WITH_AES_128_CBC_SHA256, 0x0040)                                                                        \
	XX(DHE_RSA_WITH_AES_128_CBC_SHA256, 0x0067)                                                                        \
	XX(DH_DSS_WITH_AES_256_CBC_SHA256, 0x0068)                                                                         \
	XX(DH_RSA_WITH_AES_256_CBC_SHA256, 0x0069)                                                                         \
	XX(DHE_DSS_WITH_AES_256_CBC_SHA256, 0x006A)                                                                        \
	XX(DHE_RSA_WITH_AES_256_CBC_SHA256, 0x006B)                                                                        \
	XX(DH_anon_WITH_RC4_128_MD5, 0x0018)                                                                               \
	XX(DH_anon_WITH_3DES_EDE_CBC_SHA, 0x001B)                                                                          \
	XX(DH_anon_WITH_AES_128_CBC_SHA, 0x0034)                                                                           \
	XX(DH_anon_WITH_AES_256_CBC_SHA, 0x003A)                                                                           \
	XX(DH_anon_WITH_AES_128_CBC_SHA256, 0x006C)                                                                        \
	XX(DH_anon_WITH_AES_256_CBC_SHA256, 0x006D)                                                                        \
	/* From, RFC, 4492, */                                                                                             \
	XX(ECDH_ECDSA_WITH_NULL_SHA, 0xC001)                                                                               \
	XX(ECDH_ECDSA_WITH_RC4_128_SHA, 0xC002)                                                                            \
	XX(ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA, 0xC003)                                                                       \
	XX(ECDH_ECDSA_WITH_AES_128_CBC_SHA, 0xC004)                                                                        \
	XX(ECDH_ECDSA_WITH_AES_256_CBC_SHA, 0xC005)                                                                        \
	XX(ECDHE_ECDSA_WITH_NULL_SHA, 0xC006)                                                                              \
	XX(ECDHE_ECDSA_WITH_RC4_128_SHA, 0xC007)                                                                           \
	XX(ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA, 0xC008)                                                                      \
	XX(ECDHE_ECDSA_WITH_AES_128_CBC_SHA, 0xC009)                                                                       \
	XX(ECDHE_ECDSA_WITH_AES_256_CBC_SHA, 0xC00A)                                                                       \
	XX(ECDH_RSA_WITH_NULL_SHA, 0xC00B)                                                                                 \
	XX(ECDH_RSA_WITH_RC4_128_SHA, 0xC00C)                                                                              \
	XX(ECDH_RSA_WITH_3DES_EDE_CBC_SHA, 0xC00D)                                                                         \
	XX(ECDH_RSA_WITH_AES_128_CBC_SHA, 0xC00E)                                                                          \
	XX(ECDH_RSA_WITH_AES_256_CBC_SHA, 0xC00F)                                                                          \
	XX(ECDHE_RSA_WITH_NULL_SHA, 0xC010)                                                                                \
	XX(ECDHE_RSA_WITH_RC4_128_SHA, 0xC011)                                                                             \
	XX(ECDHE_RSA_WITH_3DES_EDE_CBC_SHA, 0xC012)                                                                        \
	XX(ECDHE_RSA_WITH_AES_128_CBC_SHA, 0xC013)                                                                         \
	XX(ECDHE_RSA_WITH_AES_256_CBC_SHA, 0xC014)                                                                         \
	XX(ECDH_anon_WITH_NULL_SHA, 0xC015)                                                                                \
	XX(ECDH_anon_WITH_RC4_128_SHA, 0xC016)                                                                             \
	XX(ECDH_anon_WITH_3DES_EDE_CBC_SHA, 0xC017)                                                                        \
	XX(ECDH_anon_WITH_AES_128_CBC_SHA, 0xC018)                                                                         \
	XX(ECDH_anon_WITH_AES_256_CBC_SHA, 0xC019)                                                                         \
	/* From, RFC, 5288, */                                                                                             \
	XX(RSA_WITH_AES_128_GCM_SHA256, 0x009C)                                                                            \
	XX(RSA_WITH_AES_256_GCM_SHA384, 0x009D)                                                                            \
	XX(DHE_RSA_WITH_AES_128_GCM_SHA256, 0x009E)                                                                        \
	XX(DHE_RSA_WITH_AES_256_GCM_SHA384, 0x009F)                                                                        \
	XX(DH_RSA_WITH_AES_128_GCM_SHA256, 0x00A0)                                                                         \
	XX(DH_RSA_WITH_AES_256_GCM_SHA384, 0x00A1)                                                                         \
	XX(DHE_DSS_WITH_AES_128_GCM_SHA256, 0x00A2)                                                                        \
	XX(DHE_DSS_WITH_AES_256_GCM_SHA384, 0x00A3)                                                                        \
	XX(DH_DSS_WITH_AES_128_GCM_SHA256, 0x00A4)                                                                         \
	XX(DH_DSS_WITH_AES_256_GCM_SHA384, 0x00A5)                                                                         \
	XX(DH_anon_WITH_AES_128_GCM_SHA256, 0x00A6)                                                                        \
	XX(DH_anon_WITH_AES_256_GCM_SHA384, 0x00A7)                                                                        \
	/* From, RFC, 5289, */                                                                                             \
	XX(ECDHE_ECDSA_WITH_AES_128_CBC_SHA256, 0xC023)                                                                    \
	XX(ECDHE_ECDSA_WITH_AES_256_CBC_SHA384, 0xC024)                                                                    \
	XX(ECDH_ECDSA_WITH_AES_128_CBC_SHA256, 0xC025)                                                                     \
	XX(ECDH_ECDSA_WITH_AES_256_CBC_SHA384, 0xC026)                                                                     \
	XX(ECDHE_RSA_WITH_AES_128_CBC_SHA256, 0xC027)                                                                      \
	XX(ECDHE_RSA_WITH_AES_256_CBC_SHA384, 0xC028)                                                                      \
	XX(ECDH_RSA_WITH_AES_128_CBC_SHA256, 0xC029)                                                                       \
	XX(ECDH_RSA_WITH_AES_256_CBC_SHA384, 0xC02A)                                                                       \
	XX(ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, 0xC02B)                                                                    \
	XX(ECDHE_ECDSA_WITH_AES_256_GCM_SHA384, 0xC02C)                                                                    \
	XX(ECDH_ECDSA_WITH_AES_128_GCM_SHA256, 0xC02D)                                                                     \
	XX(ECDH_ECDSA_WITH_AES_256_GCM_SHA384, 0xC02E)                                                                     \
	XX(ECDHE_RSA_WITH_AES_128_GCM_SHA256, 0xC02F)                                                                      \
	XX(ECDHE_RSA_WITH_AES_256_GCM_SHA384, 0xC030)                                                                      \
	XX(ECDH_RSA_WITH_AES_128_GCM_SHA256, 0xC031)                                                                       \
	XX(ECDH_RSA_WITH_AES_256_GCM_SHA384, 0xC032)                                                                       \
	/* From, RFC, 6655, and, 7251, */                                                                                  \
	XX(RSA_WITH_AES_128_CCM, 0xC09C)                                                                                   \
	XX(RSA_WITH_AES_256_CCM, 0xC09D)                                                                                   \
	XX(RSA_WITH_AES_128_CCM_8, 0xC0A0)                                                                                 \
	XX(RSA_WITH_AES_256_CCM_8, 0xC0A1)                                                                                 \
	XX(ECDHE_ECDSA_WITH_AES_128_CCM, 0xC0AC)                                                                           \
	XX(ECDHE_ECDSA_WITH_AES_256_CCM, 0xC0AD)                                                                           \
	XX(ECDHE_ECDSA_WITH_AES_128_CCM_8, 0xC0AE)                                                                         \
	XX(ECDHE_ECDSA_WITH_AES_256_CCM_8, 0xC0AF)                                                                         \
	/* From, RFC, 7905, */                                                                                             \
	XX(ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256, 0xCCA8)                                                                \
	XX(ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256, 0xCCA9)                                                              \
	XX(DHE_RSA_WITH_CHACHA20_POLY1305_SHA256, 0xCCAA)                                                                  \
	XX(PSK_WITH_CHACHA20_POLY1305_SHA256, 0xCCAB)                                                                      \
	XX(ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256, 0xCCAC)                                                                \
	XX(DHE_PSK_WITH_CHACHA20_POLY1305_SHA256, 0xCCAD)                                                                  \
	XX(RSA_PSK_WITH_CHACHA20_POLY1305_SHA256, 0xCCAE)                                                                  \
	/* From, RFC, 7507, */                                                                                             \
	XX(FALLBACK_SCSV, 0x5600)

/**
 * @brief Cipher suite identifier
 *
 * The TLS standard specifies codes using two 8-bit values.
 * We combine these into a single 16-bit value in MSB-LSB order.
 *
 * For example:
 *
 * TLS_RSA_WITH_AES_128_CBC_SHA = { 0x00, 0x2F } = 0x002F
 */
enum class CipherSuite : uint16_t {
#define XX(tag, code) tag = code,
	SSL_CIPHER_SUITE_MAP(XX)
#undef XX
};

/**
 * @brief Gets the name of the cipher suite
 * @param Cipher Suite identifier
 * @retval String
 */
String getCipherSuiteName(CipherSuite id);

/** @} */

} // namespace Ssl
