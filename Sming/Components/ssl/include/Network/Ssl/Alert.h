/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Alert.h
 *
 ****/

#pragma once

#include <WString.h>

namespace Ssl
{
/**
 * @brief Map of standard Alert codes
 */
#define SSL_ALERT_CODE_MAP(XX)                                                                                         \
	XX(CLOSE_NOTIFY, 0)                                                                                                \
	XX(UNEXPECTED_MESSAGE, 10)                                                                                         \
	XX(BAD_RECORD_MAC, 20)                                                                                             \
	XX(RECORD_OVERFLOW, 22)                                                                                            \
	XX(DECOMPRESSION_FAILURE, 30)                                                                                      \
	XX(HANDSHAKE_FAILURE, 40)                                                                                          \
	XX(BAD_CERTIFICATE, 42)                                                                                            \
	XX(UNSUPPORTED_CERTIFICATE, 43)                                                                                    \
	XX(CERTIFICATE_REVOKED, 44)                                                                                        \
	XX(CERTIFICATE_EXPIRED, 45)                                                                                        \
	XX(CERTIFICATE_UNKNOWN, 46)                                                                                        \
	XX(ILLEGAL_PARAMETER, 47)                                                                                          \
	XX(UNKNOWN_CA, 48)                                                                                                 \
	XX(ACCESS_DENIED, 49)                                                                                              \
	XX(DECODE_ERROR, 50)                                                                                               \
	XX(DECRYPT_ERROR, 51)                                                                                              \
	XX(INVALID_VERSION, 70)                                                                                            \
	XX(INSUFFICIENT_SECURITY, 71)                                                                                      \
	XX(INTERNAL_ERROR, 80)                                                                                             \
	XX(USER_CANCELLED, 90)                                                                                             \
	XX(NO_RENEGOTIATION, 100)                                                                                          \
	XX(UNSUPPORTED_EXTENSION, 110)                                                                                     \
	XX(NO_APPLICATION_PROTOCOL, 120)

/**
 * @brief Alert codes defined by the standard
 */
enum class Alert {
	Invalid = -1,
#define XX(tag, code) tag = code,
	SSL_ALERT_CODE_MAP(XX)
#undef XX
};

String getAlertString(Alert alert);

} // namespace Ssl
