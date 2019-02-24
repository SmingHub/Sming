/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpConnection.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_H_

#include "HttpConnectionBase.h"
#include "DateTime.h"
#include "Data/ObjectQueue.h"

/** @defgroup   HTTP client connection
 *  @brief      Provides http client connection
 *  @ingroup    http
 *  @{
 */

typedef ObjectQueue<HttpRequest, HTTP_REQUEST_POOL_SIZE> RequestQueue;

class HttpConnection : public HttpConnectionBase
{
public:
	HttpConnection() : HttpConnectionBase(HTTP_RESPONSE)
	{
	}

	~HttpConnection()
	{
		cleanup();

		// Free any outstanding queued requests
		while(waitingQueue.count() != 0) {
			delete waitingQueue.dequeue();
		}
	}

	bool connect(const String& host, int port, bool useSsl = false, uint32_t sslOptions = 0) override;

	bool send(HttpRequest* request);

	bool isActive();

	/**
	 * @brief Returns pointer to the current request
	 * @return HttpRequest*
	 */
	HttpRequest* getRequest()
	{
		return incomingRequest;
	}

	/**
	 * @brief Returns pointer to the current response
	 * @return HttpResponse*
	 */
	HttpResponse* getResponse()
	{
		return &response;
	}

	using TcpClient::close;

#ifdef ENABLE_SSL
	using TcpClient::getSsl;
#endif

	// Backported for compatibility reasons

	/**
	 * @deprecated Use `getResponse()->code` instead
	 */
	int getResponseCode() SMING_DEPRECATED
	{
		return response.code;
	}

	/**
	 * @deprecated Use `getResponse()->headers[headerName]` instead
	 */
	String getResponseHeader(String headerName, String defaultValue = nullptr) SMING_DEPRECATED;

	/**
	* @deprecated Use `getResponse()->headers` instead
	*/
	HttpHeaders& getResponseHeaders() SMING_DEPRECATED
	{
		return response.headers;
	}

	/**
	* @todo deprecate: Use `getResponse()->headers[HTTP_HEADER_LAST_MODIFIED]` instead
	*/
	DateTime getLastModifiedDate();

	/**
	 * @todo deprecate: Use `getResponse()->headers[HTTP_HEADER_DATE]` instead
	 */
	DateTime getServerDate();

	/**
	 * @deprecated Use `getResponse()->getBody()` instead
	 */
	String getResponseString() SMING_DEPRECATED
	{
		return response.getBody();
	}

	void reset() override;

protected:
	// HTTP parser methods

	int onMessageBegin(http_parser* parser) override;
	int onHeadersComplete(const HttpHeaders& headers) override;
	int onBody(const char* at, size_t length) override;
	int onMessageComplete(http_parser* parser) override;

	// TCP methods
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;

	void cleanup() override;

private:
	void sendRequestHeaders(HttpRequest* request);
	bool sendRequestBody(HttpRequest* request);
	HttpPartResult multipartProducer();

private:
	RequestQueue waitingQueue;   ///< Requests waiting to be started
	RequestQueue executionQueue; ///< Requests being executed in a pipeline

	HttpRequest* incomingRequest = nullptr;
	HttpRequest* outgoingRequest = nullptr;
	HttpResponse response;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_H_ */
