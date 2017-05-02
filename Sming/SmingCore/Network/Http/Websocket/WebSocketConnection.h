/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_
#define SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_

#include "../../TcpServer.h"
#include "../HttpServerConnection.h"
#include "../../Services/cWebsocket/websocket.h"
extern "C" {
	#include "../ws_parser/ws_parser.h"
}

class WebSocketConnection;

typedef Vector<WebSocketConnection> WebSocketsList;

typedef Delegate<void(WebSocketConnection&)> WebSocketDelegate;
typedef Delegate<void(WebSocketConnection&, const String&)> WebSocketMessageDelegate;
typedef Delegate<void(WebSocketConnection&, uint8_t* data, size_t size)> WebSocketBinaryDelegate;

enum WsConnectionState
{
	eWSCS_Ready,
	eWSCS_Open,
	eWSCS_Closed
};

class WebSocketConnection
{
public:
	WebSocketConnection(HttpServerConnection* conn);
	virtual ~WebSocketConnection();

	bool initialize(HttpRequest &request, HttpResponse &response);

	virtual void send(const char* message, int length, wsFrameType type = WS_TEXT_FRAME);
	void broadcast(const char* message, int length, wsFrameType type = WS_TEXT_FRAME);

	void sendString(const String& message);
	void sendBinary(const uint8_t* data, int size);
	void close();

	void setUserData(void* userData);
	void* getUserData();

// @deprecated
	bool operator==(const WebSocketConnection &rhs) const;

	WebSocketsList& getActiveWebSockets();
// @end deprecated

	void setConnectionHandler(WebSocketDelegate handler);
	void setMessageHandler(WebSocketMessageDelegate handler);
	void setBinaryHandler(WebSocketBinaryDelegate handler);
	void setDisconnectionHandler(WebSocketDelegate handler);

	int processFrame(HttpServerConnection& connection, HttpRequest& request, char *at, int size);

protected:
	bool is(HttpServerConnection* conn) { return connection == conn; };

	static int staticOnDataBegin(void* userData, ws_frame_type_t type);
	static int staticOnDataPayload(void* userData, const char *at, size_t length);
	static int staticOnDataEnd(void* userData);
	static int staticOnControlBegin(void* userData, ws_frame_type_t type);
	static int staticOnControlPayload(void* userData, const char*, size_t length);
	static int staticOnControlEnd(void* userData);

protected:
	WebSocketDelegate wsConnect = 0;
	WebSocketMessageDelegate wsMessage = 0;
	WebSocketBinaryDelegate wsBinary = 0;
	WebSocketDelegate wsDisconnect = 0;

private:
	WsConnectionState state = eWSCS_Ready;

	void *userData = nullptr;
	HttpServerConnection* connection = nullptr;

	ws_frame_type_t frameType = WS_FRAME_TEXT;
	ws_frame_type_t controlFrameType = WS_FRAME_PING;

	ws_parser_t parser;
	ws_parser_callbacks_t parserSettings;

// @deprecated
	static WebSocketsList websocketList;
// @end deprecated
};

#endif /* SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_ */
