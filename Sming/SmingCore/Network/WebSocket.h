/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_WEBSOCKET_H_
#define SMINGCORE_NETWORK_WEBSOCKET_H_

#include "TcpServer.h"
#include "HttpServerConnection.h"
#include "../../Wiring/WHashMap.h"
#include "../../Wiring/WVector.h"
#include "../Delegate.h"

class HttpServer;

class WebSocket
{
	friend class HttpServer;
public:
	WebSocket(HttpServerConnection* conn);
	virtual void send(const char* message, int length);
	void sendString(const String& message);

protected:
	bool initialize(HttpRequest &request, HttpResponse &response);
	bool is(HttpServerConnection* conn) { return connection == conn; }

private:
	HttpServerConnection* connection;
};

#endif /* SMINGCORE_NETWORK_WEBSOCKET_H_ */
