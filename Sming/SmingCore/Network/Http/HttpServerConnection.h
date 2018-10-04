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

#ifndef _SMING_CORE_HTTPSERVERCONNECTION_H_
#define _SMING_CORE_HTTPSERVERCONNECTION_H_

#include "../TcpClient.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Delegate.h"

#include "HttpResource.h"
#include "HttpRequest.h"
#include "HttpBodyParser.h"

#ifndef HTTP_SERVER_EXPOSE_NAME
#define HTTP_SERVER_EXPOSE_NAME 1
#endif

#ifndef HTTP_SERVER_EXPOSE_DATE
#define HTTP_SERVER_EXPOSE_DATE 0
#endif

class HttpServerConnection;

typedef Delegate<void(HttpServerConnection& connection)> HttpServerConnectionDelegate;

class HttpServerConnection : public TcpClient
{
public:
	HttpServerConnection(tcp_pcb* clientTcp);
	virtual ~HttpServerConnection();

	void setResourceTree(ResourceTree* resourceTree);
	void setBodyParsers(BodyParsers* bodyParsers);

	void send();

	using TcpClient::send;

	using TcpConnection::getRemoteIp;
	using TcpConnection::getRemotePort;

protected:
	virtual err_t onReceive(pbuf* buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void sendError(const String& message = nullptr, enum http_status code = HTTP_STATUS_BAD_REQUEST);
	virtual void onError(err_t err);

private:
	static int staticOnMessageBegin(http_parser* parser);
	static int staticOnPath(http_parser* parser, const char* at, size_t length);
	static int staticOnHeadersComplete(http_parser* parser);
	static int staticOnHeaderField(http_parser* parser, const char* at, size_t length);
	static int staticOnHeaderValue(http_parser* parser, const char* at, size_t length);
	static int staticOnBody(http_parser* parser, const char* at, size_t length);
	static int staticOnMessageComplete(http_parser* parser);

	void sendResponseHeaders(HttpResponse* response);
	bool sendResponseBody(HttpResponse* response);

public:
	void* userData = NULL; // << use to pass user data between requests

private:
	HttpConnectionState state;

	http_parser parser;
	static http_parser_settings parserSettings;
	static bool parserSettingsInitialized;

	ResourceTree* resourceTree = NULL;
	HttpResource* resource = NULL;

	HttpRequest request = HttpRequest(URL());
	HttpResponse response;

	HttpResourceDelegate headersCompleteDelegate = 0;
	HttpResourceDelegate requestCompletedDelegate = 0;
	HttpServerConnectionBodyDelegate onBodyDelegate = 0;

	HttpHeaders requestHeaders;
	bool lastWasValue = true;
	String lastData = "";
	String currentField = "";

	BodyParsers* bodyParsers = NULL;
	HttpBodyParserDelegate bodyParser;
};

#endif /* _SMING_CORE_HTTPSERVERCONNECTION_H_ */
