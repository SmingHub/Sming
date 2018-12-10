/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpConnection
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_CONNECTION_H_
#define _SMING_CORE_NETWORK_HTTP_CONNECTION_H_

#include "HttpConnectionBase.h"
#include <DateTime/DateTime.h>
#include "Data/ObjectQueue.h"

/** @defgroup   HTTP client connection
 *  @brief      Provides http client connection
 *  @ingroup    http
 *  @{
 */

typedef ObjectQueue<HttpRequest, HTTP_REQUEST_POOL_SIZE> RequestQueue;

class HttpConnection : public HttpConnectionBase
{
	friend class HttpClient;

public:
	HttpConnection(RequestQueue* queue);
	~HttpConnection();

	bool connect(const String& host, int port, bool useSsl = false, uint32_t sslOptions = 0);

	bool send(HttpRequest* request);

	bool isActive();

	/**
	 * @brief Returns pointer to the current request
	 * @return HttpRequest*
	 */
	HttpRequest* getRequest();

	/**
	 * @brief Returns pointer to the current response
	 * @return HttpResponse*
	 */
	HttpResponse* getResponse();

	using TcpClient::close;

#ifdef ENABLE_SSL
	using TcpClient::getSsl;
#endif

	// Backported for compatibility reasons
	// @deprecated
	/**
	 * @deprecated Use `getResponse().code` instead
	 */
	__forceinline int getResponseCode()
	{
		return response.code;
	}

	/**
	 * @deprecated Use `getResponse().headers[headerName]` instead
	 */
	String getResponseHeader(String headerName, String defaultValue = nullptr);

	/**
	* @deprecated Use `getResponse().headers` instead
	*/
	HttpHeaders& getResponseHeaders();

	/**
	* @deprecated Use `getResponse().headers[HTTP_HEADER_LAST_MODIFIED]` instead
	*/
	DateTime getLastModifiedDate(); // Last-Modified header

	/**
	 * @deprecated Use `getResponse().headers[HTTP_HEADER_DATE]` instead
	 */
	DateTime getServerDate(); // Date header

	/**
	 * @deprecated Use `getResponse().stream` instead
	 */
	String getResponseString();
	// @enddeprecated

	virtual void reset();

protected:
	// HTTP parser methods

	/**
	 * Called when a new incoming data is beginning to come
	 * @paran http_parser* parser
	 * @return 0 on success, non-0 on error
	 */
	virtual int onMessageBegin(http_parser* parser);

	/**
	 * Called when all headers are received
	 * @param HttpHeaders headers - the processed headers
	 * @return 0 on success, non-0 on error
	 */
	virtual int onHeadersComplete(const HttpHeaders& headers);

	/**
	 * Called when a piece of body data is received
	 * @param const char* at -  the data
	 * @paran size_t length
	 * @return 0 on success, non-0 on error
	 */
	virtual int onBody(const char* at, size_t length);

	/**
	 * Called when the incoming data is complete
	 * @paran http_parser* parser
	 * @return 0 on success, non-0 on error
	 */
	virtual int onMessageComplete(http_parser* parser);

	// TCP methods
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);

	virtual void cleanup();

private:
	void sendRequestHeaders(HttpRequest* request);
	bool sendRequestBody(HttpRequest* request);
	HttpPartResult multipartProducer();

protected:
	RequestQueue* waitingQueue = nullptr; ///< Requests waiting to be started - we do not own this queue
	RequestQueue executionQueue;		  ///< Requests being executed in a pipeline

	HttpRequest* incomingRequest = nullptr;
	HttpRequest* outgoingRequest = nullptr;
	HttpResponse response;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTP_CONNECTION_H_ */
