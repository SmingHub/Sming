/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClientConnection.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_H_

#include "HttpConnection.h"
#include "DateTime.h"
#include "Data/ObjectQueue.h"

/** @defgroup   HTTP client connection
 *  @brief      Provides http client connection
 *  @ingroup    http
 *  @{
 */

typedef ObjectQueue<HttpRequest, HTTP_REQUEST_POOL_SIZE> RequestQueue;

class HttpClientConnection : public HttpConnection
{
public:
	HttpClientConnection() : HttpConnection(HTTP_RESPONSE)
	{
	}

	~HttpClientConnection()
	{
		cleanup();

		// Free any outstanding queued requests
		while(waitingQueue.count() != 0) {
			delete waitingQueue.dequeue();
		}

#ifdef ENABLE_SSL
		delete sslSessionId;
#endif
	}

	bool connect(const String& host, int port, bool useSsl = false, uint32_t sslOptions = 0) override;

	bool send(HttpRequest* request) override;

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
	int getResponseCode() const SMING_DEPRECATED
	{
		return response.code;
	}

	/**
	 * @deprecated Use `getResponse()->headers[]` instead
	 */
	String getResponseHeader(const String& headerName, const String& defaultValue = nullptr) const SMING_DEPRECATED
	{
		return response.headers[headerName] ?: defaultValue;
	}

	/**
	* @deprecated Use `getResponse()->headers` instead
	*/
	HttpHeaders& getResponseHeaders() SMING_DEPRECATED
	{
		return response.headers;
	}

	/**
	* @deprecated Use `getResponse()->headers.getLastModifiedDate()` instead
	*/
	DateTime getLastModifiedDate() const SMING_DEPRECATED
	{
		return response.headers.getLastModifiedDate();
	}

	/**
	 * @deprecated Use `getResponse()->headers.getServerDate()` instead
	 */
	DateTime getServerDate() const SMING_DEPRECATED
	{
		return response.headers.getServerDate();
	}

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
