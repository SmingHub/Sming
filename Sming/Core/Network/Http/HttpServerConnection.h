/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServerConnection.h
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpConnection.h"
#include "HttpResource.h"
#include "HttpBodyParser.h"

#include <functional>

/** @defgroup   HTTP server connection
 *  @brief      Provides http server connection
 *  @ingroup    http
 *  @{
 */

#ifndef HTTP_SERVER_EXPOSE_NAME
#define HTTP_SERVER_EXPOSE_NAME 1
#endif

#ifndef HTTP_SERVER_EXPOSE_DATE
#define HTTP_SERVER_EXPOSE_DATE 0
#endif

class HttpResourceTree;
class HttpServerConnection;

typedef Delegate<void(HttpServerConnection& connection)> HttpServerConnectionDelegate;

typedef Delegate<bool()> HttpServerProtocolUpgradeCallback;

class HttpServerConnection : public HttpConnection
{
public:
	HttpServerConnection(tcp_pcb* clientTcp) : HttpConnection(clientTcp, HTTP_REQUEST)
	{
	}

	~HttpServerConnection()
	{
		if(this->resource != nullptr) {
			this->resource->shutdown(*this);
		}
	}

	void setResourceTree(HttpResourceTree* resourceTree)
	{
		this->resourceTree = resourceTree;
	}

	void setBodyParsers(const BodyParsers* bodyParsers)
	{
		this->bodyParsers = bodyParsers;
	}

	void send()
	{
		state = eHCS_StartSending;
		onReadyToSendData(eTCE_Received);
	}

	using TcpClient::send;

	void setUpgradeCallback(HttpServerProtocolUpgradeCallback callback)
	{
		upgradeCallback = callback;
	}

	HttpRequest* getRequest() override
	{
		return &request;
	}

protected:
	// HTTP parser methods

	int onMessageBegin(http_parser* parser) override;
	int onPath(const Url& path) override;
	int onHeadersComplete(const HttpHeaders& headers) override;
	int onBody(const char* at, size_t length) override;
	int onMessageComplete(http_parser* parser) override;

	bool onProtocolUpgrade(http_parser* parser) override
	{
		if(upgradeCallback) {
			return upgradeCallback();
		}

		return true;
	}

	void onHttpError(http_errno error) override;

	// TCP methods
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;
	virtual void sendError(const String& message = nullptr, enum http_status code = HTTP_STATUS_BAD_REQUEST);

private:
	void sendResponseHeaders(HttpResponse* response);
	bool sendResponseBody(HttpResponse* response);

public:
	void* userData = nullptr; ///< use to pass user data between requests

private:
	HttpResourceTree* resourceTree = nullptr; ///< A reference to the current resource tree - we don't own it
	HttpResource* resource = nullptr;		  ///< Resource for currently executing path

	HttpRequest request;

	HttpResourceDelegate headersCompleteDelegate = nullptr;
	HttpResourceDelegate requestCompletedDelegate = nullptr;
	HttpServerConnectionBodyDelegate onBodyDelegate = nullptr;
	HttpServerProtocolUpgradeCallback upgradeCallback = nullptr;

	const BodyParsers* bodyParsers = nullptr;	///< const reference ensures we cannot modify map, only look stuff up
	HttpBodyParserDelegate bodyParser = nullptr; ///< Active body parser for this message, if any
};

/** @} */
