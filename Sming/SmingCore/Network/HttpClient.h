/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClient
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/** @defgroup   httpclient HTTP client
 *  @brief      Provides HTTP/S client
 *  @ingroup    tcpclient
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_HTTPCLIENT_H_
#define _SMING_CORE_NETWORK_HTTPCLIENT_H_

#include "TcpClient.h"
#include "Http/HttpCommon.h"
#include "Http/HttpRequest.h"
#include "Http/HttpConnection.h"
#include "ObjectMap.h"

class HttpClient {
public:
	/* High-Level Method */

	bool sendRequest(const String& url, RequestCompletedDelegate requestComplete)
	{
		auto req = createRequest(url);
		req->onRequestComplete(requestComplete);
		return send(req);
	}

	bool sendRequest(const HttpMethod method, const String& url, const HttpHeaders& headers,
					 RequestCompletedDelegate requestComplete)
	{
		auto req = createRequest(url);
		req->setMethod(method);
		req->headers = headers;
		req->onRequestComplete(requestComplete);
		return send(req);
	}

	bool sendRequest(const HttpMethod method, const String& url, const HttpHeaders& headers, const String& body,
					 RequestCompletedDelegate requestComplete)
	{
		auto req = createRequest(url);
		req->setMethod(method);
		req->headers = headers;
		req->setBody(body);
		req->onRequestComplete(requestComplete);
		return send(req);
	}

	bool downloadString(const String& url, RequestCompletedDelegate requestComplete);

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

	HttpRequest* createRequest(const String& url)
	{
		return new HttpRequest(URL(url));
	}


	/**
	 * Use this method to clean all request queues and object pools
	 */
	static void cleanup();

	virtual ~HttpClient();

protected:
	String getCacheKey(const URL& url);

protected:
	static ObjectMap<HttpConnection> _connectionPool;
	static ObjectMap<RequestQueue> _requestQueueMap;

#ifdef ENABLE_SSL
	static ObjectMap<SSLSessionId> _sslSessionIdPool;
#endif
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTPCLIENT_H_ */
