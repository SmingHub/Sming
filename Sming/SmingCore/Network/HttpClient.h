/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClient.h
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/** @defgroup   httpclient HTTP client
 *  @brief      Provides HTTP/S client
 *  @ingroup    tcpclient
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_HTTP_CLIENT_H_
#define _SMING_CORE_NETWORK_HTTP_CLIENT_H_

#include "TcpClient.h"
#include "Http/HttpCommon.h"
#include "Http/HttpRequest.h"
#include "Http/HttpClientConnection.h"

class HttpClient
{
public:
	/**
	 * @brief HttpClient destructor
	 * @note DON'T call cleanup.
	 * 	If you want to free all resources from HttpClients the correct sequence will be to
	 * 		1. Delete all instances of HttpClient
	 * 		2. Call the static method HttpClient::cleanup();
	*/
	virtual ~HttpClient()
	{
	}

	/* High-Level Methods */

	bool sendRequest(const String& url, RequestCompletedDelegate requestComplete)
	{
		return send(createRequest(url)->setMethod(HTTP_GET)->onRequestComplete(requestComplete));
	}

	bool sendRequest(const HttpMethod method, const String& url, const HttpHeaders& headers,
					 RequestCompletedDelegate requestComplete)
	{
		return send(createRequest(url)->setMethod(method)->setHeaders(headers)->onRequestComplete(requestComplete));
	}

	bool sendRequest(const HttpMethod method, const String& url, const HttpHeaders& headers, const String& body,
					 RequestCompletedDelegate requestComplete)
	{
		return send(createRequest(url)->setMethod(method)->setHeaders(headers)->setBody(body)->onRequestComplete(
			requestComplete));
	}

	bool downloadString(const String& url, RequestCompletedDelegate requestComplete)
	{
		return send(createRequest(url)->setMethod(HTTP_GET)->onRequestComplete(requestComplete));
	}

	bool downloadFile(const String& url, RequestCompletedDelegate requestComplete = nullptr)
	{
		return downloadFile(url, nullptr, requestComplete);
	}

	bool downloadFile(const String& url, const String& saveFileName,
					  RequestCompletedDelegate requestComplete = nullptr);

	/* Low Level Methods */

	/*
	 * @brief This method queues a request and sends it, once it is connected to the remote server.
	 * @param HttpRequest* request The request object will be freed inside of the method.
	 *                             Do not try to reuse it outside of the send method as it will lead to unpredicted results
	 *
	 * @retval bool true if the request was queued, false otherwise.
	 *
	 */
	bool send(HttpRequest* request);

	/** @deprecated Use `createRequest()` instead */
	HttpRequest* request(const String& url) SMING_DEPRECATED
	{
		return createRequest(url);
	}

	/** @brief Helper function to create a new request on a URL
	 *  @param url
	 *  @retval HttpRequest*
	 */
	HttpRequest* createRequest(const String& url)
	{
		return new HttpRequest(URL(url));
	}

	/**
	 * Use this method to clean all request queues and object pools
	 */
	static void cleanup()
	{
		httpConnectionPool.clear();
	}

protected:
	String getCacheKey(URL url)
	{
		return url.Host + ':' + url.Port;
	}

protected:
	static ObjectMap<String, HttpClientConnection> httpConnectionPool;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTP_CLIENT_H_ */
