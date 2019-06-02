/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Url.h
 *
 * @author Feb 2019 mikee47 <mike@sillyhouse.net>
 *
 * Support added for escaped URLs and standard schemes.
 *
 * @see https://en.m.wikipedia.org/wiki/URL
 * @see https://en.m.wikipedia.org/wiki/URL_normalization
 *
 ****/

/** @defgroup   url Url
 *  @brief      Provides URL handling
 *  @ingroup    httpserver
 *  @ingroup    httpclient
 *  @{
 */

#pragma once

#include "WString.h"
#include "Http/HttpParams.h"

/*
 * Common URL schemes (name, scheme, port)
 * 	name: 	Used for string identifier
 * 	scheme:	String to be used in a URL, normalised (i.e. lower case)
 * 	port: 	Offical default port definition for the scheme, 0 if not applicable
 */
#define URI_SCHEME_MAP(XX)                                                                                             \
	XX(HTTP, http, 80)                                                                                                 \
	XX(HTTP_SECURE, https, 443)                                                                                        \
	XX(WEBSOCKET, ws, 80)                                                                                              \
	XX(WEBSOCKET_SECURE, wss, 443)                                                                                     \
	XX(MQTT, mqtt, 1883)                                                                                               \
	XX(MQTT_SECURE, mqtts, 8883)                                                                                       \
	XX(SMTP, smtp, 25)                                                                                                 \
	XX(SMTP_SECURE, smtps, 465)                                                                                        \
	XX(FTP, ftp, 21)                                                                                                   \
	XX(MAIL_TO, mailto, 0)                                                                                             \
	XX(FILE, file, 0)                                                                                                  \
	XX(DATA, data, 0)

/**
 * @brief Common URI scheme strings
 */
#define XX(name, str, port) DECLARE_FSTR(URI_SCHEME_##name)
URI_SCHEME_MAP(XX)
#undef XX

class Print;

/** @brief Class to manage URL instance
 *  @note The various URL components are stored in un-escaped format for ease of editing.
 *  Unless otherwise indicated, methods act on un-escaped text.
 *  Methods used to obtain escaped versions are clearly marked.
 *  Any attached fragment (marked bv '#') in the URL is discarded
 */
class Url
{
public:
	Url() = default;

	Url(const Url& url) = default;

	/** @brief Construct a URL object from a regular escaped string
	 *  @urlString Escaped URL
	 */
	Url(const String& urlString)
	{
		*this = urlString;
	}

	/** @brief Construct a URL object from a regular null-terminated escaped string
	 *  @urlString Escaped URL
	 */
	Url(const char* urlString) : Url(String(urlString))
	{
	}

	Url(const String& scheme, const String& user, const String& password, const String& host, int port = 0,
		const String& path = nullptr, const String& query = nullptr, const String& fragment = nullptr)
		: Scheme(scheme), User(user), Password(password), Host(host), Port(port), Path(path), Query(query),
		  Fragment(fragment)
	{
		Scheme.toLowerCase();
		if(Port == 0) {
			Port = getDefaultPort(Scheme);
		}
	}

	/** @brief Copy assignment operator
	 *  @param urlString Escaped URL
	 *  @note urlString is modified by so no point in passing const reference
	 */
	Url& operator=(String urlString);

	/** @brief Copy assignment operator, for C-style strings
	 *  @param urlString Escaped URL
	 */
	Url& operator=(const char* urlString)
	{
		*this = String(urlString);
		return *this;
	}

	/** @brief Get escaped URL
	 * 	@retval String
	 */
	String toString() const;

	operator String() const
	{
		return toString();
	}

	/** @brief Obtain the default port for a given scheme
	 *  @retval int 0 if scheme is not recognised or has no standard port defined
	 */
	static int getDefaultPort(const String& scheme);

	/** @brief Obtain the actual port number to be used
	 *  @retval int
	 *  @note If not specified, the default scheme port is returned
	 */
	int getPort() const
	{
		return Port ?: getDefaultPort(Scheme);
	}

	/** @brief Get hostname+port part of URL string
	 *  @retval String
	 *  @note Neither of these is subject to escaping
	 */
	String getHostWithPort() const;

	/** @brief Get path without leading separator
	 *  @retval String
	 */
	String getRelativePath() const
	{
		return (Path[0] == '/') ? Path.substring(1) : Path;
	}

	/** @brief Get path with any query parameters attached
	 *  @retval String
	 *  @note Both path and query values are escaped
	 */
	String getPathWithQuery() const;

	/** @brief Obtain the filename part of the URL path
	 *  @retval String
	 */
	String getFileName() const;

	/**
	 * @brief Printable output for debugging
	 * @param p
	 */
	void debugPrintTo(Print& p) const;

public:
	String Scheme; ///< without ":" and "//"
	String User;
	String Password;
	String Host;  ///< hostname or IP address
	int Port = 0; ///< Undefined by default
	String Path;  ///< with leading "/"
	HttpParams Query;
	String Fragment; ///< Without '#'
};

typedef Url URL SMING_DEPRECATED; ///< @deprecated Use `Url` instead

/** @} */
