/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
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

#pragma once

#include "TcpClient.h"
#include "Http/HttpCommon.h"
#include "Http/HttpRequest.h"
#include "Http/HttpClientConnection.h"
#include "Data/Stream/LimitedMemoryStream.h"

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

	bool sendRequest(const Url& url, RequestCompletedDelegate requestComplete)
	{
		return send(createRequest(url)->setMethod(HTTP_GET)->onRequestComplete(requestComplete));
	}

	bool sendRequest(const HttpMethod method, const Url& url, const HttpHeaders& headers,
					 RequestCompletedDelegate requestComplete)
	{
		return send(createRequest(url)->setMethod(method)->setHeaders(headers)->onRequestComplete(requestComplete));
	}

	bool sendRequest(const HttpMethod method, const Url& url, const HttpHeaders& headers, const String& body,
					 RequestCompletedDelegate requestComplete)
	{
		return send(createRequest(url)->setMethod(method)->setHeaders(headers)->setBody(body)->onRequestComplete(
			requestComplete));
	}

	/**
	 * @brief Queue request to download content as string (in memory)
	 * @param url URL from which the content will be fetched
	 * @param requestComplete Completion callback
	 * @param maxLength maximum bytes to store in memory. If the response is bigger than `maxLength` then the rest bytes will be discarded.
	 * 					Use this parameter wisely as setting the value too high may consume all available RAM resulting in
	 * 					device restart and Denial-Of-Service
	 */
	bool downloadString(const Url& url, RequestCompletedDelegate requestComplete,
						size_t maxLength = NETWORK_SEND_BUFFER_SIZE)
	{
		return send(createRequest(url)
						->setMethod(HTTP_GET)
						->setResponseStream(new LimitedMemoryStream(maxLength))
						->onRequestComplete(requestComplete));
	}

	bool downloadFile(const Url& url, RequestCompletedDelegate requestComplete = nullptr)
	{
		return downloadFile(url, nullptr, requestComplete);
	}

	/**
	 * @brief Queue request to download a file
	 * @param url Source of file data
	 * @param saveFileName Path to save file to. Optional: specify nullptr to use name from url
	 * @param requestComplete Completion callback
	 */
	bool downloadFile(const Url& url, const String& saveFileName, RequestCompletedDelegate requestComplete = nullptr);

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
	HttpRequest* createRequest(const Url& url)
	{
		return new HttpRequest(url);
	}

	/**
	 * Use this method to clean all request queues and object pools
	 */
	static void cleanup()
	{
		httpConnectionPool.clear();
	}

protected:
	String getCacheKey(const Url& url)
	{
		return url.Host + ':' + url.getPort();
	}

protected:
	typedef ObjectMap<String, HttpClientConnection> HttpConnectionPool;
	static HttpConnectionPool httpConnectionPool;
};

/** @} */
