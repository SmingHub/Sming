/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpServer.h"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServerConnection.h"
#include "TcpClient.h"
#include "../Wiring/WString.h"
#include "../../Libraries/cWebsocket/websocket.h"

HttpServer::HttpServer()
{
	defaultHandler = NULL;
	setTimeOut(90);

	// Default processing headers
	// Add more in you app!
	enableHeaderProcessing("Cookie");
	enableHeaderProcessing("Host");
	enableHeaderProcessing("Content-Type");
	enableHeaderProcessing("Content-Length");

	enableHeaderProcessing("Upgrade");
}

HttpServer::~HttpServer()
{
}

TcpConnection* HttpServer::createClient(tcp_pcb *clientTcp)
{
	TcpConnection* con = new HttpServerConnection(this, clientTcp);
	return con;
}

void HttpServer::enableHeaderProcessing(String headerName)
{
	for (int i = 0; i < processingHeaders.count(); i++)
		if (processingHeaders[i].equals(headerName))
			return;

	processingHeaders.add(headerName);
}

void HttpServer::addPath(String path, HttpPathDelegate callback)
{
	if (path.length() > 1 && path.endsWith("/"))
		path = path.substring(0, path.length() - 1);
	if (!path.startsWith("/"))
		path = "/" + path;
	debugf("'%s' registered", path.c_str());
	paths[path] = callback;
}

void HttpServer::setDefaultHandler(HttpPathDelegate callback)
{
	defaultHandler = callback;
}

bool HttpServer::processRequest(HttpServerConnection &connection, HttpRequest &request, HttpResponse &response)
{
	if (request.isWebSocket())
	{
		bool res = initWebSocket(connection, request, response);
		if (!res) response.badRequest();
	}
	String path = request.getPath();
	if (path.length() > 1 && path.endsWith("/"))
		path = path.substring(0, path.length() - 1);

	if (paths.contains(path))
	{
		paths[path](request, response);
		return true;
	}

	if (defaultHandler)
	{
		debugf("Default server handler for: '%s'", path.c_str());
		defaultHandler(request, response);
		return true;
	}

	debugf("ERROR at server 404: '%s' not found", path.c_str());
	return false;
}

bool HttpServer::isHeaderProcessingEnabled(String name)
{
	for (int i = 0; i < processingHeaders.count(); i++)
		if (processingHeaders[i].equals(name))
			return true;

	return false;
}

bool HttpServer::initWebSocket(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	if (!wsEnabled)
		return false;

	auto sock = WebSocket(&connection);
	if (!sock.initialize(request, response))
		return false;

	connection.setDisconnectionHandler(HttpServerConnectionDelegate(&HttpServer::onCloseWebSocket, this)); // auto remove on close
	response.sendHeader(connection); // Will push header before user data

	wsocks.add(sock);
	if (wsConnect) wsConnect(sock);

	return true;
}

void HttpServer::processWebSocketFrame(pbuf *buf, HttpServerConnection& connection)
{
	//TODO: process splitted payload
	uint8_t* data; size_t size;
	wsFrameType frameType = wsParseInputFrame((uint8_t*)buf->payload, buf->len, &data, &size);
	WebSocket* sock = getWebSocket(connection);

	if (frameType == WS_TEXT_FRAME)
	{
		String msg;
		msg.setString((char*)data, size);
		debugf("WS: %s", msg.c_str());
		if (sock && wsMessage) wsMessage(*sock, msg);
	}
	if (frameType == WS_BINARY_FRAME)
	{
		if (sock && wsMessage) wsBinary(*sock, data, size);
	}
	else if (frameType == WS_CLOSING_FRAME)
	{
		connection.close(); // it will be processed automatically in onCloseWebSocket callback
	}
	else if (frameType == WS_INCOMPLETE_FRAME || frameType == WS_ERROR_FRAME)
		debugf("WS error reading frame: %X", frameType);
	else
		debugf("WS frame type: %X", frameType);
}

void HttpServer::setWebSocketConnectionHandler(WebSocketDelegate handler)
{
	wsConnect = handler;
}

void HttpServer::setWebSocketMessageHandler(WebSocketMessageDelegate handler)
{
	wsMessage = handler;
}

void HttpServer::setWebSocketBinaryHandler(WebSocketBinaryDelegate handler)
{
	wsBinary = handler;
}

void HttpServer::setWebSocketDisconnectionHandler(WebSocketDelegate handler)
{
	wsDisconnect = handler;
}

WebSocket* HttpServer::getWebSocket(HttpServerConnection& connection)
{
	for (int i = 0; i < wsocks.count(); i++)
		if (wsocks[i].is(&connection))
			return &wsocks[i];

	return nullptr;
}

void HttpServer::removeWebSocket(HttpServerConnection& connection)
{
	debugf("WS remove connection item");
	for (int i = 0; i < wsocks.count(); i++)
		if (wsocks[i].is(&connection))
			wsocks.remove(i--);
}

void HttpServer::onCloseWebSocket(HttpServerConnection& connection)
{
	WebSocket* sock = getWebSocket(connection);

	removeWebSocket(connection);

	debugf("WS Close");
	if (sock && wsDisconnect) wsDisconnect(*sock);
}

void HttpServer::enableWebSockets(bool enabled)
{
	wsEnabled = enabled;
	if (wsEnabled)
	{
		enableHeaderProcessing("Sec-WebSocket-Key");
		enableHeaderProcessing("Sec-WebSocket-Version");
	}
}
