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

#include <functional>

WebsocketResource::WebsocketResource()
{
	onHeadersComplete = HttpResourceDelegate(&WebsocketResource::checkHeaders, this);
}

WebsocketResource::~WebsocketResource()
{
}

int WebsocketResource::checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	WebsocketConnection* socket = new WebsocketConnection(&connection, false);
	socket->setBinaryHandler(wsBinary);
	socket->setMessageHandler(wsMessage);
	socket->setConnectionHandler(wsConnect);
	socket->setDisconnectionHandler(wsDisconnect);
	if(!socket->bind(request, response)) {
		debug_w("Not a valid WebsocketRequest?");
		delete socket;
		return -1;
	}

	connection.setTimeOut(USHRT_MAX); //Disable disconnection on connection idle (no rx/tx)

	connection.setUpgdareCallback(std::bind(&WebsocketConnection::onConnected, socket));

	// TODO: Re-Enable Command Executor...

	return 0;
}

void WebsocketResource::shutdown(HttpServerConnection& connection)
{
	WebsocketConnection* socket = static_cast<WebsocketConnection*>(connection.userData);
	delete socket;
	connection.userData = NULL;
	connection.setTimeOut(1);
}

void WebsocketResource::setConnectionHandler(WebsocketDelegate handler)
{
	wsConnect = handler;
}

void WebsocketResource::setMessageHandler(WebsocketMessageDelegate handler)
{
	wsMessage = handler;
}

void WebsocketResource::setBinaryHandler(WebsocketBinaryDelegate handler)
{
	wsBinary = handler;
}

void WebsocketResource::setDisconnectionHandler(WebsocketDelegate handler)
{
	wsDisconnect = handler;
}
