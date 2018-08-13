/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   httpconsts HTTP constants to be used with HTTP client or HTTP server
 *  @brief      Provides HTTP constants
 *  @ingroup    httpserver
 *  @ingroup    httpclient
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_WEBCONSTANTS_H_
#define _SMING_CORE_NETWORK_WEBCONSTANTS_H_

#include "WString.h"

// MUST list longer extensions first
#define MIME_TYPE_MAP(XX)                                                                                              \
	/* Type, extension start, Mime type */                                                                             \
                                                                                                                       \
	/* Texts */                                                                                                        \
	XX(HTML, "html", "text/html")                                                                                      \
	XX(TEXT, "txt", "text/plain")                                                                                      \
	XX(JSON, "json", "application/json")                                                                               \
	XX(JS, "js", "text/javascript")                                                                                    \
	XX(CSS, "css", "text/css")                                                                                         \
	XX(XML, "xml", "text/xml")                                                                                         \
                                                                                                                       \
	/* Images */                                                                                                       \
	XX(JPEG, "jpg", "image/jpeg")                                                                                      \
	XX(GIF, "git", "image/gif")                                                                                        \
	XX(PNG, "png", "image/png")                                                                                        \
	XX(SVG, "svg", "image/svg+xml")                                                                                    \
	XX(ICO, "ico", "image/x-icon")                                                                                     \
                                                                                                                       \
	/* Archives */                                                                                                     \
	XX(GZIP, "gzip", "application/x-gzip")                                                                             \
	XX(ZIP, "zip", "application/zip")                                                                                  \
                                                                                                                       \
	/* Binary and Form */                                                                                              \
	XX(BINARY, "", "application/octet-stream")                                                                         \
	XX(FORM_URL_ENCODED, "", "application/x-www-form-urlencoded")                                                      \
	XX(FORM_MULTIPART, "", "multipart/form-data")

enum MimeType {
#define XX(name, extensionStart, mime) MIME_##name,
	MIME_TYPE_MAP(XX)
#undef XX
};

namespace ContentType {
String fromFileExtension(String extension);
String toString(enum MimeType m);
String fromFileName(const String& fileName);
}; // namespace ContentType

/** @} */
#endif /* _SMING_CORE_NETWORK_WEBCONSTANTS_H_ */
