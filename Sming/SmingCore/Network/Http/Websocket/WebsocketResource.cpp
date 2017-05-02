/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "WebsocketResource.h"

WebsocketResource::WebsocketResource() {
	onHeadersComplete = HttpResourceDelegate(&WebsocketResource::checkHeaders, this);
	onUpgrade = HttpServerConnectionUpgradeDelegate(&WebsocketResource::processData, this);
}

WebsocketResource::~WebsocketResource()
{
}

int WebsocketResource::checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) {
	WebSocketConnection* socket = new WebSocketConnection(&connection);
	socket->setBinaryHandler(wsBinary);
	socket->setMessageHandler(wsMessage);
	socket->setConnectionHandler(wsConnect);
	socket->setDisconnectionHandler(wsDisconnect);
	if (!socket->initialize(request, response)) {
		debugf("Not a valid WebsocketRequest?");
		delete socket;
		return -1;
	}

	connection.setTimeOut(USHRT_MAX); //Disable disconnection on connection idle (no rx/tx)
	connection.userData = (void *)socket;

// TODO: Re-Enable Command Executor...

	return 0;
}

int WebsocketResource::processData(HttpServerConnection& connection, HttpRequest& request, char *at, int size)
{
	WebSocketConnection *socket = (WebSocketConnection *)connection.userData;
	if(socket == NULL) {
		return -1;
	}

	return socket->processFrame(connection, request, at, size);
}

void WebsocketResource::setConnectionHandler(WebSocketDelegate handler) {
	wsConnect = handler;
}

void WebsocketResource::setMessageHandler(WebSocketMessageDelegate handler) {
	wsMessage = handler;
}

void WebsocketResource::setBinaryHandler(WebSocketBinaryDelegate handler) {
	wsBinary = handler;
}

void WebsocketResource::setDisconnectionHandler(WebSocketDelegate handler) {
	wsDisconnect = handler;
}
