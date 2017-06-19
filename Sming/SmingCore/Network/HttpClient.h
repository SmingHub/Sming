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

#ifndef _SMING_CORE_NETWORK_HTTPCLIENT_H_
#define _SMING_CORE_NETWORK_HTTPCLIENT_H_

#include "TcpClient.h"
#include "Http/HttpCommon.h"
#include "Http/HttpRequest.h"
#include "Http/HttpConnection.h"

class HttpClient
{
public:
	/* High-Level Method */

	__forceinline bool sendRequest(const String& url, RequestCompletedDelegate requestComplete) {
		return send(request(url)
				   ->setMethod(HTTP_GET)
				   ->onRequestComplete(requestComplete)
				   );
	}


	__forceinline bool sendRequest(const HttpMethod method, const String& url, const HttpHeaders& headers, RequestCompletedDelegate requestComplete) {
		return send(request(url)
				   ->setMethod(method)
				   ->setHeaders(headers)
				   ->onRequestComplete(requestComplete)
				   );
	}

	__forceinline bool sendRequest(const HttpMethod method, const String& url, const HttpHeaders& headers, const String& body, RequestCompletedDelegate requestComplete) {
			return send(request(url)
					   ->setMethod(method)
					   ->setHeaders(headers)
					   ->setBody(body)
					   ->onRequestComplete(requestComplete)
					   );
	}

	bool downloadString(const String& url, RequestCompletedDelegate requestComplete);

	__forceinline bool downloadFile(const String& url, RequestCompletedDelegate requestComplete = NULL) {
		return downloadFile(url, "", requestComplete);
	}

	bool downloadFile(const String& url, const String& saveFileName, RequestCompletedDelegate requestComplete = NULL);

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

	HttpRequest* request(const String& url);

#ifdef ENABLE_SSL
	static void freeSslSessionPool();
#endif

	/**
	 * Use this method to clean all request queues and object pools
	 */
	static void cleanup();

	virtual ~HttpClient();

protected:
	String getCacheKey(URL url);

protected:
	static HashMap<String, HttpConnection *> httpConnectionPool;
	static HashMap<String, RequestQueue* > queue;

#ifdef ENABLE_SSL
	static HashMap<String, SSLSessionId* > sslSessionIdPool;
#endif
};

#endif /* _SMING_CORE_NETWORK_HTTPCLIENT_H_ */
