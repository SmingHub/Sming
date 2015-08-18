/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTPSERVERCONNECTION_H_
#define _SMING_CORE_HTTPSERVERCONNECTION_H_

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpConnection.h"
#include "../Wiring/WString.h"
#include "../Delegate.h"

class HttpServer;

enum HttpConnectionState
{
	eHCS_Ready,
	eHCS_ParsePostData,
	eHCS_ParsingCompleted,
	eHCS_Sending,
	eHCS_WebSocketFrames,
	eHCS_Sent
};

typedef Delegate<void(HttpServerConnection& connection)> HttpServerConnectionDelegate;

class HttpServerConnection: public TcpConnection
{
public:
	HttpServerConnection(HttpServer *parentServer, tcp_pcb *clientTcp);
	virtual ~HttpServerConnection();

	virtual void close();
	void setDisconnectionHandler(HttpServerConnectionDelegate handler);

protected:
	virtual err_t onReceive(pbuf *buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void beginSendData();
	virtual void sendError(const char* message = NULL);

	virtual void onError(err_t err);

private:
	HttpServer *server;
	HttpConnectionState state;
	HttpRequest request;
	HttpResponse response;
	HttpServerConnectionDelegate disconnection;

	friend class HttpResponse;
	friend class HttpRequest;
};

#endif /* _SMING_CORE_HTTPSERVERCONNECTION_H_ */
