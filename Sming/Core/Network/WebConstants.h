/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebConstants.h
 *
 ****/

/** @defgroup   httpconsts HTTP constants to be used with HTTP client or HTTP server
 *  @brief      Provides HTTP constants
 *  @ingroup    httpserver
 *  @ingroup    httpclient
 *  @{
 */

#pragma once

#include "WString.h"

/** @brief Basic MIME types and file extensions
 *  @note Each MIME type can have only one associated file extension. Where other extensions
 *  @todo Consider using sz-strings for file extension to enable matching to alternative file extensions
 */
#define MIME_TYPE_MAP(XX)                                                                                              \
	/* Type, extension start, Mime type */                                                                             \
                                                                                                                       \
	/* Texts */                                                                                                        \
	XX(HTML, "html", "text/html")                                                                                      \
	XX(TEXT, "txt", "text/plain")                                                                                      \
	XX(JS, "js", "text/javascript")                                                                                    \
	XX(CSS, "css", "text/css")                                                                                         \
	XX(XML, "xml", "text/xml")                                                                                         \
	XX(JSON, "json", "application/json")                                                                               \
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

namespace ContentType
{
/** @brief Obtain content type string from file extension
 *  @param extension excluding '.' separator (e.g. "htm", "json")
 *  @retval String
 */
String fromFileExtension(const char* extension);

/** @brief Obtain content type string from file extension
 *  @param extension
 *  @retval String
 */
static inline String fromFileExtension(const String& extension)
{
	return fromFileExtension(extension.c_str());
}

/** @brief Get textual representation for a MIME type
 *  @param m the MIME type
 *  @retval String
 */
String toString(enum MimeType m);

/** @brief Obtain content type string from file name or path, with extension
 *  @param fileName
 *  @retval String
 */
String fromFullFileName(const char* fileName);

/** @brief Obtain content type string from file name or path, with extension
 *  @param fileName
 *  @retval String
 */
static inline String fromFullFileName(const String& fileName)
{
	return fromFullFileName(fileName.c_str());
}

}; // namespace ContentType

/** @} */
