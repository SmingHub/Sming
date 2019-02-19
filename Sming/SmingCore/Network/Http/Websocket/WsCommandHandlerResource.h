/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WsCommandHandlerResource.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_WEBSOCKET_WS_COMMAND_HANDLER_RESOURCE_H_
#define _SMING_CORE_NETWORK_HTTP_WEBSOCKET_WS_COMMAND_HANDLER_RESOURCE_H_

#include "../HttpResource.h"
#include "WebsocketConnection.h"
#include "WString.h"
#include "../Services/CommandProcessing/CommandProcessingIncludes.h" // TODO: ....

class WsCommandHandlerResource : protected WebsocketResource
{
public:
	WsCommandHandlerResource() : WebsocketResource()
	{
		wsMessage = WebsocketMessageDelegate(&WsCommandHandlerResource::onMessage, this);
	}

protected:
	int checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) override
	{
		int err = WebsocketResource::checkHeaders(connection, request, response);
		if(err != 0) {
			return err;
		}

		WebsocketConnection* socket = (WebsocketConnection*)connection.userData;
		if(socket != nullptr) {
			socket->setMessageHandler();

			// create new command handler
		}
	}

	void onMessage(WebsocketConnection& connection, const String& message)
	{
		commandExecutor.executorReceive(message + "\r");
	}

private:
	CommandExecutor commandExecutor;
};

#endif /* _SMING_CORE_NETWORK_HTTP_WEBSOCKET_WS_COMMAND_HANDLER_RESOURCE_H_ */
