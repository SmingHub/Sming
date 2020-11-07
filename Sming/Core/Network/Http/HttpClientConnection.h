/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClientConnection.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpConnection.h"
#include "DateTime.h"
#include "Data/ObjectQueue.h"
#include <Data/Stream/MultipartStream.h>

/**
 *  @brief      Provides http client connection
 *  @ingroup    http
 *  @{
 */

using RequestQueue = ObjectQueue<HttpRequest, HTTP_REQUEST_POOL_SIZE>;

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
	}

	bool connect(const String& host, int port, bool useSsl = false) override;

	bool send(HttpRequest* request) override;

	HttpRequest* getRequest() override
	{
		return incomingRequest;
	}

	void reset() override;

	bool isFinished()
	{
		return (waitingQueue.count() + executionQueue.count() == 0);
	}

protected:
	// HTTP parser methods

	int onMessageBegin(http_parser* parser) override;
	int onHeadersComplete(const HttpHeaders& headers) override;
	int onBody(const char* at, size_t length) override;
	int onMessageComplete(http_parser* parser) override;

	// TCP methods
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;

	void onClosed() override;

	void cleanup() override;

	void sslInitSession(Ssl::Session& session) override
	{
		TcpClient::sslInitSession(session);
		auto request = waitingQueue.peek();
		if(request != nullptr && request->sslInitDelegate) {
			request->sslInitDelegate(session, *request);
		}
	}

private:
	void sendRequestHeaders(HttpRequest* request);
	bool sendRequestBody(HttpRequest* request);
	MultipartStream::BodyPart multipartProducer();

private:
	RequestQueue waitingQueue;   ///< Requests waiting to be started
	RequestQueue executionQueue; ///< Requests being executed in a pipeline

	HttpRequest* incomingRequest = nullptr;
	HttpRequest* outgoingRequest = nullptr;
};

/** @} */
