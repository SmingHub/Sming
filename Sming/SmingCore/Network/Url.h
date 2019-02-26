/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Url.h
 *
 ****/

/** @defgroup   url Url
 *  @brief      Provides URL handling
 *  @ingroup    httpserver
 *  @ingroup    httpclient
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_URL_H_
#define _SMING_CORE_NETWORK_URL_H_

#include "WString.h"

#define DEFAULT_URL_PROTOCOL _F("http")
#define HTTPS_URL_PROTOCOL _F("https")
#define WEBSOCKET_SECURE_URL_PROTOCOL _F("wss")

class Url
{
public:
	Url();
	Url(const String& urlString);

	inline String toString()
	{
		return Protocol + "://" + Host + (Port != 0 ? ":" + String(Port) : "") + getPathWithQuery();
	}
	inline String getPathWithQuery()
	{
		if(Path.length() + Query.length() > 0)
			return Path + Query;
		else
			return "/";
	}

public:
	String Protocol;
	String User;
	String Password;
	String Host;
	int Port;
	String Path;
	String Query;
};

typedef Url URL SMING_DEPRECATED; ///< @deprecated Use `Url` instead

/** @} */
#endif /* _SMING_CORE_NETWORK_URL_H_ */
