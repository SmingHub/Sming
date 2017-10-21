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

#include "../../Wiring/WString.h"

#define DEFAULT_URL_PROTOCOL "http"
#define HTTPS_URL_PROTOCOL "https"
#define WEBSCOKET_SECURE_URL_PROTOCOL "wss"

class URL
{
public:
	URL();
	URL(const String& urlString);

	inline String toString() { return Protocol + "://" + Host + (Port != 0 ? ":" + String(Port) : "") + getPathWithQuery(); }
	inline String getPathWithQuery() { if (Path.length() + Query.length() > 0) return Path + Query; else return "/"; }

public:
	String Protocol;
	String Host;
	int Port;
	String Path;
	String Query;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_URL_H_ */
