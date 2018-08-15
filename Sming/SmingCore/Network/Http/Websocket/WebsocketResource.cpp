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

WebsocketResource::WebsocketResource()
{
	onHeadersComplete = HttpResourceDelegate(&WebsocketResource::checkHeaders, this);
	onUpgrade = HttpServerConnectionUpgradeDelegate(&WebsocketResource::processData, this);
}

WebsocketResource::~WebsocketResource()
{}

int WebsocketResource::checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	WebSocketConnection* socket = createConnection(connection);
	socket->setBinaryHandler(wsBinary);
	socket->setMessageHandler(wsMessage);
	socket->setConnectionHandler(wsConnect);
	socket->setDisconnectionHandler(wsDisconnect);
	if (!socket->initialize(request, response)) {
		debug_w("Not a valid WebsocketRequest?");
		delete socket;
		return -1;
	}

	// Note: setTimeOut moved to WebSocketConnection constructor

	// TODO: Re-Enable Command Executor...

	return 0;
}

void WebsocketResource::shutdown(HttpServerConnection& connection)
{
	WebSocketConnection* socket = (WebSocketConnection*)connection.userData;
	delete socket;
	connection.userData = nullptr;
}

int WebsocketResource::processData(HttpServerConnection& connection, HttpRequest& request, char* at, int size)
{
	WebSocketConnection* socket = (WebSocketConnection*)connection.userData;
	return socket ? socket->processFrame(connection, request, at, size) : -1;
}
