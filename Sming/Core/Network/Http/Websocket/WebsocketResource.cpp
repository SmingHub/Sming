/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebsocketResource.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "WebsocketResource.h"

#include <functional>

int WebsocketResource::checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	WebsocketConnection* socket = new WebsocketConnection(&connection, false);
	if(!socket) {
		debug_e("Unable to create websocket connection");
		return 1;
	}

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
	connection.userData = (void*)socket;
	connection.setUpgradeCallback(std::bind(&WebsocketConnection::onConnected, socket));

	// TODO: Re-Enable Command Executor...

	return 0;
}

void WebsocketResource::shutdown(HttpServerConnection& connection)
{
	WebsocketConnection* socket = static_cast<WebsocketConnection*>(connection.userData);
	delete socket;
	connection.userData = nullptr;
	connection.setTimeOut(1);
}
