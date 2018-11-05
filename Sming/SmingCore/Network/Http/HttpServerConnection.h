/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServerConnection
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTPSERVERCONNECTION_H_
#define _SMING_CORE_NETWORK_HTTP_HTTPSERVERCONNECTION_H_

#include "HttpConnectionBase.h"
#include "HttpResource.h"
#include "HttpBodyParser.h"

#include <functional>

#ifndef HTTP_SERVER_EXPOSE_NAME
#define HTTP_SERVER_EXPOSE_NAME 1
#endif

#ifndef HTTP_SERVER_EXPOSE_DATE
#define HTTP_SERVER_EXPOSE_DATE 0
#endif

class HttpServerConnection;

typedef Delegate<void(HttpServerConnection& connection)> HttpServerConnectionDelegate;

typedef std::function<bool()> HttpServerProtocolUpgradeCallback;

class HttpServerConnection : public HttpConnectionBase
{
public:
	HttpServerConnection(tcp_pcb* clientTcp);
	virtual ~HttpServerConnection();

	void setResourceTree(ResourceTree* resourceTree);
	void setBodyParsers(BodyParsers* bodyParsers);

	void send();

	using TcpClient::send;

	void setUpgdareCallback(HttpServerProtocolUpgradeCallback callback)
	{
		upgradeCallback = callback;
	}

protected:
	// HTTP parser methods
	/**
	 * Called when a new incoming data is beginning to come
	 * @paran http_parser* parser
	 * @return 0 on success, non-0 on error
	 */
	virtual int onMessageBegin(http_parser* parser);

	/**
	 * Called when the URL path is known
	 * @param String path
	 * @return 0 on success, non-0 on error
	 */
	virtual int onPath(const URL& path);

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

	virtual bool onProtocolUpgrade(http_parser* parser);

	virtual void onHttpError(http_errno error);

	// TCP methods
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void sendError(const String& message = nullptr, enum http_status code = HTTP_STATUS_BAD_REQUEST);

private:
	void sendResponseHeaders(HttpResponse* response);
	bool sendResponseBody(HttpResponse* response);

public:
	void* userData = nullptr; // << use to pass user data between requests

private:
	ResourceTree* resourceTree = nullptr;
	HttpResource* resource = nullptr;

	HttpRequest request = HttpRequest(URL());
	HttpResponse response;

	HttpResourceDelegate headersCompleteDelegate = 0;
	HttpResourceDelegate requestCompletedDelegate = 0;
	HttpServerConnectionBodyDelegate onBodyDelegate = 0;
	HttpServerProtocolUpgradeCallback upgradeCallback = nullptr;

	BodyParsers* bodyParsers = nullptr;
	HttpBodyParserDelegate bodyParser = 0;
};

#endif /* _SMING_CORE_NETWORK_HTTP_HTTPSERVERCONNECTION_H_ */
