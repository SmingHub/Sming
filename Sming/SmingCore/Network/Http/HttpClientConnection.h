/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClientConnection.h
 *
 ****/

/** @defgroup   HTTP client connection
 *  @brief      Provides HTTP/S client connection
 *  @ingroup    http
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_CLIENT_CONNECTION_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_CLIENT_CONNECTION_H_

#include "HttpConnection.h"

class HttpClientConnection : public HttpConnection
{
public:
	HttpClientConnection() : HttpConnection(&requestQueue)
	{
	}

	virtual ~HttpClientConnection();

	/** @brief Queue a request
	 *  @param request
	 *  @retval bool true on success
	 *  @note we take ownership of the request. On error, it is destroyed before returning.
	 */
	bool send(HttpRequest* request);

private:
	RequestQueue requestQueue;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTP_HTTP_CLIENT_CONNECTION_H_ */
