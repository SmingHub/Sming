/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxError.cpp
 *
 ****/

#include "AxError.h"
#include <axtls-8266/ssl/ssl.h>
#include <FlashString/Map.hpp>

#define AX_ERROR_MAP(XX)                                                                                               \
	XX(OK)                                                                                                             \
	XX(NOT_OK)                                                                                                         \
	XX(ERROR_DEAD)                                                                                                     \
	XX(CLOSE_NOTIFY)                                                                                                   \
	XX(ERROR_CONN_LOST)                                                                                                \
	XX(ERROR_RECORD_OVERFLOW)                                                                                          \
	XX(ERROR_SOCK_SETUP_FAILURE)                                                                                       \
	XX(ERROR_INVALID_HANDSHAKE)                                                                                        \
	XX(ERROR_INVALID_PROT_MSG)                                                                                         \
	XX(ERROR_INVALID_HMAC)                                                                                             \
	XX(ERROR_INVALID_VERSION)                                                                                          \
	XX(ERROR_UNSUPPORTED_EXTENSION)                                                                                    \
	XX(ERROR_INVALID_SESSION)                                                                                          \
	XX(ERROR_NO_CIPHER)                                                                                                \
	XX(ERROR_INVALID_CERT_HASH_ALG)                                                                                    \
	XX(ERROR_BAD_CERTIFICATE)                                                                                          \
	XX(ERROR_INVALID_KEY)                                                                                              \
	XX(ERROR_FINISHED_INVALID)                                                                                         \
	XX(ERROR_NO_CERT_DEFINED)                                                                                          \
	XX(ERROR_NO_CLIENT_RENOG)                                                                                          \
	XX(ERROR_NOT_SUPPORTED)

#define AX_X509_ERROR_MAP(XX)                                                                                          \
	XX(X509_NOT_OK)                                                                                                    \
	XX(X509_VFY_ERROR_NO_TRUSTED_CERT)                                                                                 \
	XX(X509_VFY_ERROR_BAD_SIGNATURE)                                                                                   \
	XX(X509_VFY_ERROR_NOT_YET_VALID)                                                                                   \
	XX(X509_VFY_ERROR_EXPIRED)                                                                                         \
	XX(X509_VFY_ERROR_SELF_SIGNED)                                                                                     \
	XX(X509_VFY_ERROR_INVALID_CHAIN)                                                                                   \
	XX(X509_VFY_ERROR_UNSUPPORTED_DIGEST)                                                                              \
	XX(X509_INVALID_PRIV_KEY)                                                                                          \
	XX(X509_MAX_CERTS)                                                                                                 \
	XX(X509_VFY_ERROR_BASIC_CONSTRAINT)

namespace Ssl
{
#define XX(tag) DEFINE_FSTR_LOCAL(errStr_##tag, #tag)
AX_ERROR_MAP(XX)
#undef XX

#define XX(tag) {SSL_##tag, &errStr_##tag},
DEFINE_FSTR_MAP_LOCAL(errorMap, int, FSTR::String, AX_ERROR_MAP(XX));
#undef XX

#define XX(tag) DEFINE_FSTR_LOCAL(x509ErrStr_##tag, #tag)
AX_X509_ERROR_MAP(XX)
#undef XX

#define XX(tag) {tag, &x509ErrStr_##tag},
DEFINE_FSTR_MAP_LOCAL(x509ErrorMap, int, FSTR::String, AX_X509_ERROR_MAP(XX));
#undef XX

String getErrorString(int error)
{
	String s;

	// X509 error ?
	if(error < SSL_X509_OFFSET) {
		s = String(x509ErrorMap[error - SSL_X509_OFFSET]);
	} else if(error < SSL_CLOSE_NOTIFY && error > SSL_ERROR_CONN_LOST) {
		auto alert = Alert(-error);
		s = getAlertString(alert);
	} else {
		s = String(errorMap[error]);
	}
	return s ?: F("Unknown_") + String(error);
}

Alert getAlert(int error)
{
	if(error == SSL_CLOSE_NOTIFY) {
		return Alert::CLOSE_NOTIFY;
	}

	if(error > SSL_ERROR_CONN_LOST && error < SSL_CLOSE_NOTIFY) {
		return Alert(-error);
	}

	return Alert::Invalid;
}

} // namespace Ssl
