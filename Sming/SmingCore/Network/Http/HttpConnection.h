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

#ifndef _SMING_CORE_HTTP_CONNECTION_H_
#define _SMING_CORE_HTTP_CONNECTION_H_

#include "HttpCommon.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "../TcpClient.h"
#include "Data/Stream/DataSourceStream.h"
#include "Data/Stream/MultipartStream.h"
#include "../Services/DateTime/DateTime.h"
#include "Data/HttpHeaders.h"
#include "ObjectQueue.h"

typedef ObjectQueue<HttpRequest, HTTP_REQUEST_POOL_SIZE> RequestQueue;

class HttpConnection : public TcpClient {
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
	HttpRequest* getRequest()
	{
		return _incomingRequest;
	}

	/**
	 * @brief Returns pointer to the current response
	 * @return HttpResponse*
	 */
	HttpResponse* getResponse()
	{
		return &_response;
	}

	using TcpConnection::getRemoteIp;
	using TcpConnection::getRemotePort;

	using TcpClient::close;

#ifdef ENABLE_SSL
	using TcpClient::getSsl;
#endif

	// Backported for compatibility reasons
	// @deprecated
	/**
	 * @deprecated Use `getResponse().code` instead
	 */
	int getResponseCode()
	{
		return _response.code;
	}

	/**
	* @deprecated Use `getResponse().headers` instead
	*/
	HttpHeaders& getResponseHeaders()
	{
		return _response.headers;
	}

	/**
	* @deprecated Use `getResponse().headers["Last-Modified"]` instead
	*/
	DateTime getLastModifiedDate(); // Last-Modified header

	/**
	 * @deprecated Use `getResponse().headers["Date"]` instead
	 */
	DateTime getServerDate(); // Date header

	/**
	 * @deprecated Use `getResponse().stream` instead
	 */
	String getResponseString()
	{
		return _response.getBody();
	}

	// @enddeprecated

protected:
	void reset();

	virtual err_t onReceive(pbuf* buf);
	virtual err_t onProtocolUpgrade();
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void onError(err_t err);

	void cleanup();

private:
	HTTP_PARSER_METHOD_0(HttpConnection, on_message_begin)
#ifndef COMPACT_MODE
	HTTP_PARSER_METHOD_2(HttpConnection, on_status)
#endif
	HTTP_PARSER_METHOD_2(HttpConnection, on_header_field)
	HTTP_PARSER_METHOD_2(HttpConnection, on_header_value)
	HTTP_PARSER_METHOD_0(HttpConnection, on_headers_complete)
	HTTP_PARSER_METHOD_2(HttpConnection, on_body)
	HTTP_PARSER_METHOD_0(HttpConnection, on_message_complete)
#ifndef COMPACT_MODE
	HTTP_PARSER_METHOD_0(HttpConnection, on_chunk_header)
	HTTP_PARSER_METHOD_0(HttpConnection, on_chunk_complete)
#endif

protected:
	RequestQueue* _waitingQueue = nullptr; ///< Requests waiting to be started - we do not own this queue
	RequestQueue _executionQueue;		   ///< Requests being executed in a pipeline

	/* Incoming headers are parsed using a third-party library. The data is
	 * received in callbacks but can be fragmented. We therefore need to
	 * store some information to re-assemble the correct data before
	 * adding to headers.
	 */
	http_parser _parser;
	bool _lastWasValue = true;
	String _lastData;
	HttpHeaderFieldName _currentField = hhfn_UNKNOWN;

	//
	HttpRequest* _incomingRequest = nullptr;
	//
	HttpRequest* _outgoingRequest = nullptr;
	//
	HttpResponse _response;

private:
	//
	HttpConnectionState _state = eHCS_Ready;

private:
	HttpPartResult multipartProducer();
};

#endif /* _SMING_CORE_HTTP_CONNECTION_H_ */
