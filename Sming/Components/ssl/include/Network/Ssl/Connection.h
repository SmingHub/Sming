/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Connection.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include <user_config.h>
#include "SessionId.h"
#include "Certificate.h"

namespace Ssl
{
/**
 * @ingroup ssl
 * @brief Provides the functionality needed to create SSL communicate over TCP
 * @{
 */

/* errors that can be generated - these are specific to AXTLS */
#define SSL_OK 0
#define SSL_NOT_OK -1
#define SSL_ERROR_DEAD -2
#define SSL_CLOSE_NOTIFY -3
#define SSL_ERROR_CONN_LOST -256
#define SSL_ERROR_RECORD_OVERFLOW -257
#define SSL_ERROR_SOCK_SETUP_FAILURE -258
#define SSL_ERROR_INVALID_HANDSHAKE -260
#define SSL_ERROR_INVALID_PROT_MSG -261
#define SSL_ERROR_INVALID_HMAC -262
#define SSL_ERROR_INVALID_VERSION -263
#define SSL_ERROR_UNSUPPORTED_EXTENSION -264
#define SSL_ERROR_INVALID_SESSION -265
#define SSL_ERROR_NO_CIPHER -266
#define SSL_ERROR_INVALID_CERT_HASH_ALG -267
#define SSL_ERROR_BAD_CERTIFICATE -268
#define SSL_ERROR_INVALID_KEY -269
#define SSL_ERROR_FINISHED_INVALID -271
#define SSL_ERROR_NO_CERT_DEFINED -272
#define SSL_ERROR_NO_CLIENT_RENOG -273
#define SSL_ERROR_NOT_SUPPORTED -274
#define SSL_X509_OFFSET -512

#define SSL_ALERT_CODE_MAP(XX)                                                                                         \
	XX(CLOSE_NOTIFY, 0)                                                                                                \
	XX(UNEXPECTED_MESSAGE, 10)                                                                                         \
	XX(BAD_RECORD_MAC, 20)                                                                                             \
	XX(RECORD_OVERFLOW, 22)                                                                                            \
	XX(HANDSHAKE_FAILURE, 40)                                                                                          \
	XX(BAD_CERTIFICATE, 42)                                                                                            \
	XX(UNSUPPORTED_CERTIFICATE, 43)                                                                                    \
	XX(CERTIFICATE_EXPIRED, 45)                                                                                        \
	XX(CERTIFICATE_UNKNOWN, 46)                                                                                        \
	XX(ILLEGAL_PARAMETER, 47)                                                                                          \
	XX(UNKNOWN_CA, 48)                                                                                                 \
	XX(DECODE_ERROR, 50)                                                                                               \
	XX(DECRYPT_ERROR, 51)                                                                                              \
	XX(INVALID_VERSION, 70)                                                                                            \
	XX(NO_RENEGOTIATION, 100)                                                                                          \
	XX(UNSUPPORTED_EXTENSION, 110)

/**
 * @brief Alert codes defined by the standard
 */
enum AlertCode {
#define XX(tag, code) SSL_ALERT##tag = code,
	SSL_ALERT_CODE_MAP(XX)
#undef XX
};

/*
 * Cipher suites
 *
 *	TLS v1.2	https://tools.ietf.org/html/rfc5246#appendix-A.5
 * 	TLS v1.3	https://tools.ietf.org/html/rfc8446#appendix-B.4
 *
 */
#define SSL_CIPHER_SUITE_MAP(XX)                                                                                       \
	XX(0x00, 0x2f, TLS_RSA_WITH_AES_128_CBC_SHA)                                                                       \
	XX(0x00, 0x35, TLS_RSA_WITH_AES_256_CBC_SHA)                                                                       \
	XX(0x00, 0x3c, TLS_RSA_WITH_AES_128_CBC_SHA256)                                                                    \
	XX(0x00, 0x3d, TLS_RSA_WITH_AES_256_CBC_SHA256)                                                                    \
	XX(0x13, 0x01, TLS_AES_128_GCM_SHA256)                                                                             \
	XX(0x13, 0x02, TLS_AES_256_GCM_SHA384)                                                                             \
	XX(0x13, 0x03, TLS_CHACHA20_POLY1305_SHA256)                                                                       \
	XX(0x13, 0x04, TLS_AES_128_CCM_SHA256)                                                                             \
	XX(0x13, 0x05, TLS_AES_128_CCM_8_SHA256)

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
enum class CipherSuite {
#define XX(n1, n2, tag) tag = (n1 << 8) | n2,
	SSL_CIPHER_SUITE_MAP(XX)
#undef XX
};

/**
 * @brief Gets the name of the cipher suite
 * @param Cipher Suite identifier
 * @retval String
 */
String cipherSuiteName(CipherSuite id);

class Connection : public Printable
{
public:
	virtual ~Connection()
	{
	}

	/**
	 * @brief Checks if the handshake has finished
	 * @retval bool true on success
	 */
	virtual bool isHandshakeDone() = 0;

	/**
	 * @brief Reads encrypted information and decrypts it
	 * @param tcp active tcp connection
	 * @param encrypted Source encrypted data
	 * @param decrypted Decrypted plaintext
	 *
	 * @retval
	 * 		 0 - when the handshake is still in progress
	 * 		 > 0 - when the is decrypted data
	 * 		 < 0 - in case of an error
	 */
	virtual int read(tcp_pcb* tcp, pbuf* encrypted, pbuf*& decrypted) = 0;

	/**
	 * @brief Converts and sends plaintext data
	 * @param data
	 * @param length
	 * @retval int length of the data that was actually written
	 */
	virtual int write(const uint8_t* data, size_t length) = 0;

	/**
	 * @brief Calculates the size of encrypted plain text data
	 * @param plainTextLength
	 * @retval int
	 */
	virtual int calcWriteSize(size_t plainTextLength) const = 0;

	/**
	 * @brief Gets the cipher suite that was used
	 * @retval CipherSuite IDs as defined by SSL/TLS standard
	 */
	virtual CipherSuite getCipherSuite() const = 0;

	/**
	 * @brief Gets the current session id object.
	 *        Should be called after handshake.
	 * @retval SessionId
	 */
	virtual SessionId getSessionId() const = 0;

	/**
	 * @brief Gets the certificate object.
	 *        That object MUST be owned by the Ssl::Connection implementation
	 *        and should not be freed outside of it
	 *
	 * @retval Ssl::SessionId*
	 */
	virtual const Certificate& getCertificate() const = 0;

	/**
	 * @brief For debugging
	 */
	size_t printTo(Print& p) const override;
};

/** @} */

} // namespace Ssl
