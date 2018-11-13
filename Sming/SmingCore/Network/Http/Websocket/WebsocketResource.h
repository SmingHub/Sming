/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_
#define _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_

#include "../HttpServerConnection.h"
#include "../HttpResource.h"
#include "WebsocketConnection.h"
#include "../../Wiring/WString.h"

class WebsocketResource : public HttpResource
{
public:
	WebsocketResource();
	~WebsocketResource();
	int checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);

	virtual void shutdown(HttpServerConnection& connection);

	void setConnectionHandler(WebsocketDelegate handler);
	void setMessageHandler(WebsocketMessageDelegate handler);
	void setBinaryHandler(WebsocketBinaryDelegate handler);
	void setDisconnectionHandler(WebsocketDelegate handler);

protected:
	bool onConnect();

protected:
	WebsocketDelegate wsConnect = 0;
	WebsocketMessageDelegate wsMessage = 0;
	WebsocketBinaryDelegate wsBinary = 0;
	WebsocketDelegate wsDisconnect = 0;
};

#endif /* _SMING_SMINGCORE_NETWORK_WEBSOCKET_RESOURCE_H_ */
