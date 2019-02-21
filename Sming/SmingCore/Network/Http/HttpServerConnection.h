/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServerConnection.h
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_SERVER_CONNECTION_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_SERVER_CONNECTION_H_

#include "HttpConnectionBase.h"
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

class ResourceTree;
class HttpServerConnection;

typedef Delegate<void(HttpServerConnection& connection)> HttpServerConnectionDelegate;

typedef std::function<bool()> HttpServerProtocolUpgradeCallback;

class HttpServerConnection : public HttpConnectionBase
{
public:
	HttpServerConnection(tcp_pcb* clientTcp) : HttpConnectionBase(clientTcp, HTTP_REQUEST)
	{
	}

	~HttpServerConnection()
	{
		if(this->resource != nullptr) {
			this->resource->shutdown(*this);
		}
	}

	void setResourceTree(ResourceTree* resourceTree)
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

protected:
	// HTTP parser methods
	/**
	 * Called when a new incoming data is beginning to come
	 * @paran http_parser* parser
	 * @return 0 on success, non-0 on error
	 */
	int onMessageBegin(http_parser* parser) override;

	/**
	 * Called when the URL path is known
	 * @param String path
	 * @return 0 on success, non-0 on error
	 */
	int onPath(const URL& path) override;

	/**
	 * Called when all headers are received
	 * @param HttpHeaders headers - the processed headers
	 * @return 0 on success, non-0 on error
	 */
	int onHeadersComplete(const HttpHeaders& headers) override;

	/**
	 * Called when a piece of body data is received
	 * @param const char* at -  the data
	 * @paran size_t length
	 * @return 0 on success, non-0 on error
	 */
	int onBody(const char* at, size_t length) override;

	/**
	 * Called when the incoming data is complete
	 * @paran http_parser* parser
	 * @return 0 on success, non-0 on error
	 */
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
	ResourceTree* resourceTree = nullptr; ///< A reference to the current resource tree - we don't own it
	HttpResource* resource = nullptr;	 ///< Resource for currently executing path

	HttpRequest request;
	HttpResponse response;

	HttpResourceDelegate headersCompleteDelegate = nullptr;
	HttpResourceDelegate requestCompletedDelegate = nullptr;
	HttpServerConnectionBodyDelegate onBodyDelegate = nullptr;
	HttpServerProtocolUpgradeCallback upgradeCallback = nullptr;

	const BodyParsers* bodyParsers = nullptr;	///< const reference ensures we cannot modify map, only look stuff up
	HttpBodyParserDelegate bodyParser = nullptr; ///< Active body parser for this message, if any
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTP_HTTP_SERVER_CONNECTION_H_ */
