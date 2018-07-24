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
extern "C" {
#include "../ws_parser/ws_parser.h"
}

enum wsState { WS_STATE_OPENING, WS_STATE_NORMAL, WS_STATE_CLOSING };

class WebsocketConnection;

typedef Vector<WebsocketConnection*> WebSocketsList;

typedef Delegate<void(WebsocketConnection&)> WebsocketDelegate;
typedef Delegate<void(WebsocketConnection&, const String&)> WebsocketMessageDelegate;
typedef Delegate<void(WebsocketConnection&, uint8_t* data, size_t size)> WebsocketBinaryDelegate;

enum WsConnectionState { eWSCS_Ready, eWSCS_Open, eWSCS_Closed };

typedef struct {
	ws_frame_type_t type;
	char* payload;
	size_t payloadLegth;
} WsFrameInfo;

class WebsocketConnection
{
public:
	static const char* secret;

public:
	virtual ~WebsocketConnection();

	/**
	 * @brief Binds websocket connection to an http server connection
	 */
	bool bind(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);

	virtual void send(const char* message, int length, ws_frame_type_t type = WS_FRAME_TEXT);
	static void broadcast(const char* message, int length, ws_frame_type_t type = WS_FRAME_TEXT);

	void sendString(const String& message);
	void sendBinary(const uint8_t* data, int size);
	void close();

	void setUserData(void* userData);
	void* getUserData();

	// @deprecated
	bool operator==(const WebsocketConnection& rhs) const;

	WebSocketsList& getActiveWebSockets();
	// @end deprecated

	void setConnectionHandler(WebsocketDelegate handler);
	void setMessageHandler(WebsocketMessageDelegate handler);
	void setBinaryHandler(WebsocketBinaryDelegate handler);
	void setDisconnectionHandler(WebsocketDelegate handler);

	int processFrame(HttpServerConnection& connection, HttpRequest& request, char* at, int size);

protected:
	static int staticOnDataBegin(void* userData, ws_frame_type_t type);
	static int staticOnDataPayload(void* userData, const char* at, size_t length);
	static int staticOnDataEnd(void* userData);
	static int staticOnControlBegin(void* userData, ws_frame_type_t type);
	static int staticOnControlPayload(void* userData, const char*, size_t length);
	static int staticOnControlEnd(void* userData);

	int encodeFrame(ws_frame_type_t type, const char* inData, size_t inLength, char* outData, size_t outLength,
					bool useMask = true, bool isFin = true);

protected:
	WebsocketDelegate wsConnect = 0;
	WebsocketMessageDelegate wsMessage = 0;
	WebsocketBinaryDelegate wsBinary = 0;
	WebsocketDelegate wsDisconnect = 0;

	EndlessMemoryStream* stream = NULL;
	void* userData = nullptr;

private:
	WsConnectionState state = eWSCS_Ready;

	ws_frame_type_t frameType = WS_FRAME_TEXT;
	WsFrameInfo controlFrame;

	ws_parser_t parser;
	ws_parser_callbacks_t parserSettings;

	static WebSocketsList websocketList;

	bool isClientConnection = true;

	HttpServerConnection* serverConnection = nullptr;
};

#endif /* SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_ */
