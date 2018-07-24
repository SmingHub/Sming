/*
 * WebResource.h
 *
 *  Created on: Apr 3, 2017
 *      Author: slavey
 */

#ifndef _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_
#define _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_

#include "../HttpResource.h"
#include "WebSocketConnection.h"
#include "../../Wiring/WString.h"
#include "../../Services/CommandProcessing/CommandProcessingIncludes.h" // TODO: ....

class WsCommandHandlerResource : protected WebsocketResource
{
public:
	WsCommandHandlerResource() : WebsocketResource()
	{
		wsMessage = WebSocketMessageDelegate(&WsCommandHandlerResource::onMessage, this);
	}

protected:
	int checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
	{
		int err = WebsocketResource::checkHeaders(connection, request, response);
		if(err != 0) {
			return err;
		}

		WebSocketConnection* socket = (WebSocketConnection*)connection.userData;
		if(socket != NULL) {
			socket->setMessageHandler()

			// create new command handler
		}
	}

	void onMessage(WebSocketConnection& connection, const String& message)
	{
		commandExecutor.executorReceive(message + "\r");
	}

private:
	CommandExecutor commandExecutor;
};

#endif /* _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_ */
