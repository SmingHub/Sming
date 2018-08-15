/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   url URL
 *  @brief      Provides URL handling
 *  @ingroup    httpserver
 *  @ingroup    httpclient
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_URL_H_
#define _SMING_CORE_NETWORK_URL_H_

#include "WString.h"
#include "../Services/WebHelpers/escape.h"
#include "IPAddress.h"
#include "Data/HttpParams.h"

// Common protocol strings
#define DEFAULT_URL_PROTOCOL _F("http")
#define HTTPS_URL_PROTOCOL _F("https")
#define WEBSCOKET_SECURE_URL_PROTOCOL _F("wss")
#define SMTP_PROTOCOL _F("smtp")
#define SMTP_OVER_SSL_PROTOCOL _F("smtps")

class URL {
public:
	URL()
	{}
	URL(String urlString);

	~URL()
	{
		delete _queryParams;
	}

	String toString() const;

	const String& protocol() const
	{
		return _protocol;
	}

	const String& user() const
	{
		return _user;
	}

	const String& password() const
	{
		return _password;
	}

	const String& host() const
	{
		return _host;
	}

	void setHost(const String& host)
	{
		_host = host;
	}

	void setHost(const IPAddress& ipaddr)
	{
		_host = ipaddr.toString();
	}

	int port() const
	{
		return _port;
	}

	void setPort(uint16_t port)
	{
		_port = port;
	}

	String hostWithPort()
	{
		return host() + ':' + port();
	}

	String path() const
	{
		return uri_unescape(_path);
	}

	/**
	 * @brief Get path without leading '/'
	 */
	String relativePath() const
	{
		String s = path();
		if (s[0] == '/')
			s.remove(0, 1);
		return s;
	}

	String query() const
	{
		return uri_unescape(_query);
	}

	HttpParams& QueryParameters();

	String pathWithQuery() const;

	String fileName() const;

private:
	//  Set all members to nullptr so if() will return false by default
	String _protocol = nullptr;
	//
	String _user = nullptr;
	String _password = nullptr;
	//
	String _host = nullptr;
	uint16_t _port = 80;
	// Stored in escaped format
	String _path = nullptr;
	String _query = nullptr;
	// << deprecated
	HttpParams* _queryParams = nullptr;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_URL_H_ */
