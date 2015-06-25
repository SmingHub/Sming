/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTPSERVER_H_
#define _SMING_CORE_HTTPSERVER_H_

#include "TcpServer.h"
#include "WebSocket.h"
#include "../../Wiring/WHashMap.h"
#include "../../Wiring/WVector.h"
#include "../Delegate.h"

class String;
class HttpServerConnection;
class HttpRequest;
class HttpResponse;

typedef Vector<WebSocket> WebSocketsList;

typedef Delegate<void(HttpRequest&, HttpResponse&)> HttpPathDelegate;
typedef Delegate<void(WebSocket&)> WebSocketDelegate;
typedef Delegate<void(WebSocket&, const String&)> WebSocketMessageDelegate;
typedef Delegate<void(WebSocket&, uint8_t* data, size_t size)> WebSocketBinaryDelegate;

class HttpServer: public TcpServer
{
	friend class HttpServerConnection;
public:
	HttpServer();
	virtual ~HttpServer();

	void enableHeaderProcessing(String headerName);
	bool isHeaderProcessingEnabled(String name);

	void addPath(String path, HttpPathDelegate callback);
	void setDefaultHandler(HttpPathDelegate callback);

	/// Web Sockets
	void enableWebSockets(bool enabled);
	__forceinline WebSocketsList& getActiveWebSockets() { return wsocks; }
	void setWebSocketConnectionHandler(WebSocketDelegate handler);
	void setWebSocketMessageHandler(WebSocketMessageDelegate handler);
	void setWebSocketBinaryHandler(WebSocketBinaryDelegate handler);
	void setWebSocketDisconnectionHandler(WebSocketDelegate handler);

protected:
	virtual TcpConnection* createClient(tcp_pcb *clientTcp);
	virtual bool initWebSocket(HttpServerConnection &connection, HttpRequest &request, HttpResponse &response);
	virtual bool processRequest(HttpServerConnection &connection, HttpRequest &request, HttpResponse &response);
	virtual void processWebSocketFrame(pbuf *buf, HttpServerConnection &connection);

	WebSocket* getWebSocket(HttpServerConnection &connection);
	void removeWebSocket(HttpServerConnection &connection);
	void onCloseWebSocket(HttpServerConnection &connection);

private:
	HttpPathDelegate defaultHandler;
	Vector<String> processingHeaders;
	HashMap<String, HttpPathDelegate> paths;
	WebSocketsList wsocks;

	bool wsEnabled = false;
	WebSocketDelegate wsConnect;
	WebSocketMessageDelegate wsMessage;
	WebSocketBinaryDelegate wsBinary;
	WebSocketDelegate wsDisconnect;
};

#endif /* _SMING_CORE_HTTPSERVER_H_ */
