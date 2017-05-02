/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTPSERVERCONNECTION_H_
#define _SMING_CORE_HTTPSERVERCONNECTION_H_

#include "../TcpClient.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Delegate.h"

#include "HttpResource.h"
#include "HttpRequest.h"

#ifndef HTTP_SERVER_EXPOSE_NAME
#define HTTP_SERVER_EXPOSE_NAME 1
#endif

#ifndef HTTP_SERVER_EXPOSE_DATE
#define HTTP_SERVER_EXPOSE_DATE 0
#endif

class HttpServerConnection;

typedef Delegate<void(HttpServerConnection& connection)> HttpServerConnectionDelegate;

enum HttpConnectionState
{
	eHCS_Ready,
	eHCS_Sending,
	eHCS_Sent
};

class HttpServerConnection: public TcpClient
{
public:
	HttpServerConnection(tcp_pcb *clientTcp);
	virtual ~HttpServerConnection();

	void setResourceTree(ResourceTree* resourceTree);

	void send();

	using TcpClient::send;

//	virtual void close();

protected:
	virtual err_t onReceive(pbuf *buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void sendError(const char* message = NULL, enum http_status code = HTTP_STATUS_BAD_REQUEST);
	virtual void onError(err_t err);

	const char * getStatus(enum http_status s);

private:
	static int IRAM_ATTR staticOnMessageBegin(http_parser* parser);
	static int IRAM_ATTR staticOnPath(http_parser *parser, const char *at, size_t length);
	static int IRAM_ATTR staticOnHeadersComplete(http_parser* parser);
	static int IRAM_ATTR staticOnHeaderField(http_parser *parser, const char *at, size_t length);
	static int IRAM_ATTR staticOnHeaderValue(http_parser *parser, const char *at, size_t length);
	static int IRAM_ATTR staticOnBody(http_parser *parser, const char *at, size_t length);
	static int IRAM_ATTR staticOnMessageComplete(http_parser* parser);

public:
	void* userData = NULL; // << use to pass user data between requests

private:
	HttpConnectionState state;

	http_parser parser;
	http_parser_settings parserSettings;

	ResourceTree* resourceTree = NULL;
	HttpResource* resource = NULL;

	HttpRequest request = HttpRequest(URL());
	HttpResponse response;

	bool headersSent = false;

	HttpResourceDelegate headersCompleteDelegate = 0;
	HttpResourceDelegate requestCompletedDelegate = 0;
	HttpServerConnectionBodyDelegate onBodyDelegate = 0;

	HttpHeaders requestHeaders;
	bool lastWasValue = true;
	String lastData = "";
	String currentField  = "";
};

#endif /* _SMING_CORE_HTTPSERVERCONNECTION_H_ */
