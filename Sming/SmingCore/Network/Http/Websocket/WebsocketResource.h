/*
 * WebResource.h
 *
 *  Created on: Apr 3, 2017
 *      Author: slavey
 */

/*
 * 17/7/2018 (mikee47)
 *
 * 	createConnection method added to allow connection customisation
 */

#ifndef _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_
#define _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_

#include "../HttpResource.h"
#include "WebSocketConnection.h"
#include "WString.h"

class WebsocketResource : public HttpResource {
public:
	WebsocketResource();
	~WebsocketResource();
	int checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);
	int processData(HttpServerConnection& connection, HttpRequest& request, char* at, int size);

	virtual void shutdown(HttpServerConnection& connection);

	void setConnectionHandler(WebSocketDelegate handler)
	{
		wsConnect = handler;
	}

	void setMessageHandler(WebSocketMessageDelegate handler)
	{
		wsMessage = handler;
	}

	void setBinaryHandler(WebSocketBinaryDelegate handler)
	{
		wsBinary = handler;
	}

	void setDisconnectionHandler(WebSocketDelegate handler)
	{
		wsDisconnect = handler;
	}

protected:
	WebSocketDelegate wsConnect = 0;
	WebSocketMessageDelegate wsMessage = 0;
	WebSocketBinaryDelegate wsBinary = 0;
	WebSocketDelegate wsDisconnect = 0;

protected:
	/** @brief Inherited classes can override this to customise connection behaviour */
	virtual WebSocketConnection* createConnection(HttpServerConnection& connection)
	{
		return new WebSocketConnection(connection);
	}
};

#endif /* _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_ */
