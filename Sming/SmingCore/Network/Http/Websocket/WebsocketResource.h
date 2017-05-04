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

class WebsocketResource: public HttpResource {

public:
	WebsocketResource();
	~WebsocketResource();
	int checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);
	int processData(HttpServerConnection& connection, HttpRequest& request, char *at, int size);

	void setConnectionHandler(WebSocketDelegate handler);
	void setMessageHandler(WebSocketMessageDelegate handler);
	void setBinaryHandler(WebSocketBinaryDelegate handler);
	void setDisconnectionHandler(WebSocketDelegate handler);

protected:
	WebSocketDelegate wsConnect = 0;
	WebSocketMessageDelegate wsMessage = 0;
	WebSocketBinaryDelegate wsBinary = 0;
	WebSocketDelegate wsDisconnect = 0;
};

#endif /* _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_ */
