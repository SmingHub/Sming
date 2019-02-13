/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_
#define SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_

#include "Network/TcpServer.h"
#include "../HttpConnectionBase.h"
#include "Data/Stream/EndlessMemoryStream.h"
extern "C" {
#include "../ws_parser/ws_parser.h"
}

/** @defgroup   Websocket connection
 *  @brief      Provides websocket connection (server and client)
 *  @ingroup    websocket http
 *  @{
 */

#define WEBSOCKET_VERSION 13 // 1.3

#define WSSTR_CONNECTION _F("connection")
#define WSSTR_UPGRADE _F("upgrade")
#define WSSTR_WEBSOCKET _F("websocket")
#define WSSTR_HOST _F("host")
#define WSSTR_ORIGIN _F("origin")
#define WSSTR_KEY _F("Sec-WebSocket-Key")
#define WSSTR_PROTOCOL _F("Sec-WebSocket-Protocol")
#define WSSTR_VERSION _F("Sec-WebSocket-Version")
#define WSSTR_SECRET _F("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")

class WebsocketConnection;

typedef Vector<WebsocketConnection*> WebsocketList;

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
	/**
	 * @brief Constructs a websocket connection on top of http client or server connection
	 * @param HttpConnectionBase* connection the transport connection
	 * @param bool isClientConnection true when the passed connection is an http client conneciton
	 */
	WebsocketConnection(HttpConnectionBase* connection, bool isClientConnection = true);

	virtual ~WebsocketConnection();

	/**
	 * @brief Binds websocket connection to an http server connection
	 * @param HttpRequest& request
	 * @param HttpResponse& response
	 * @retval true on success, false otherwise
	 */
	bool bind(HttpRequest& request, HttpResponse& response);

	/**
	 * @brief Sends a websocket message
	 * @param const char* message
	 * @param  int length
	 * @param  ws_frame_type_t type
	 */
	virtual void send(const char* message, int length, ws_frame_type_t type = WS_FRAME_TEXT);

	/**
	 * @brief Broadcasts a message to all active websocket connections
	 * @param const char* message
	 * @param  int length
	 * @param  ws_frame_type_t type
	 */
	static void broadcast(const char* message, int length, ws_frame_type_t type = WS_FRAME_TEXT);

	/**
	 * @brief Sends a string websocket message
	 * @param const String& message
	 */
	void sendString(const String& message);

	/**
	 * @brief Sends a binary websocket message
	 * @param const uint8_t* data
	 * @param int length
	 */
	void sendBinary(const uint8_t* data, int length);

	/**
	 * @brief Closes a websocket connection (without closing the underlying http connection
	 */
	void close();

	/**
	 * @brief Resets a websocket connection
	 */
	void reset();

	/**
	 * @brief Attaches a user data to a websocket connection
	 * @param void* userData
	 */
	void setUserData(void* userData);

	/**
	 * @brief Retrieves user data attached
	 * @retval void*
	 */
	void* getUserData();

	// @deprecated
	bool operator==(const WebsocketConnection& rhs) const __deprecated;

	WebsocketList& getActiveWebsockets() __deprecated;
	// @end deprecated

	/**
	 * @brief Sets the callback handler to be called after successful websocket connection
	 * @param WebsocketDelegate handler
	 */
	void setConnectionHandler(WebsocketDelegate handler);

	/**
	 * @brief Sets the callback handler to be called after a websocket message is received
	 * @param WebsocketMessageDelegate handler
	 */
	void setMessageHandler(WebsocketMessageDelegate handler);

	/**
	 * @brief Sets the callback handler to be called after a binary websocket message is received
	 * @param WebsocketBinaryDelegate handler
	 */
	void setBinaryHandler(WebsocketBinaryDelegate handler);

	/**
	 * @brief Sets the callback handler to be called before closing a websocket connection
	 * @param WebsocketDelegate handler
	 */
	void setDisconnectionHandler(WebsocketDelegate handler);

	/**
	 * @brief Should be called after a websocket connection is established to activate
	 * 		  the websocket parser and allow sending of websocket data
	 */
	void activate();

	/**
	 * @brief Call this method when the websocket connection was (re)activated.
	 * @retval bool true on success
	 */
	bool onConnected();

	/**
	 * @brief Gets the underlying HTTP connection
	 * @retval HttpConnectionBase
	 */
	HttpConnectionBase* getConnection()
	{
		return connection;
	}

	/**
	 * @brief Sets the underlying (transport ) HTTP connection
	 * @param HttpConnectionBase* connection the transport connection
	 * @param bool isClientConnection true when the passed connection is an http client conneciton
	 */
	void setConnection(HttpConnectionBase* connection, bool isClientConnection = true)
	{
		this->connection = connection;
		this->isClientConnection = isClientConnection;
	}

	/** @brief  Gets the state of the websocket connection
	  * @retval WsConnectionState
	  */
	WsConnectionState getState()
	{
		return state;
	}

protected:
	static int staticOnDataBegin(void* userData, ws_frame_type_t type);
	static int staticOnDataPayload(void* userData, const char* at, size_t length);
	static int staticOnDataEnd(void* userData);
	static int staticOnControlBegin(void* userData, ws_frame_type_t type);
	static int staticOnControlPayload(void* userData, const char*, size_t length);
	static int staticOnControlEnd(void* userData);

	bool processFrame(TcpClient& client, char* at, int size);

	size_t encodeFrame(ws_frame_type_t type, const char* inData, size_t inLength, char* outData, size_t outLength,
					   bool useMask = true, bool isFin = true);

protected:
	WebsocketDelegate wsConnect = 0;
	WebsocketMessageDelegate wsMessage = 0;
	WebsocketBinaryDelegate wsBinary = 0;
	WebsocketDelegate wsDisconnect = 0;

	void* userData = nullptr;

	WsConnectionState state = eWSCS_Ready;

private:
	ws_frame_type_t frameType = WS_FRAME_TEXT;
	WsFrameInfo controlFrame;

	ws_parser_t parser;
	ws_parser_callbacks_t parserSettings;

	static WebsocketList websocketList;

	bool isClientConnection = true;

	HttpConnectionBase* connection = nullptr;
	bool activated = false;
};

/** @} */
#endif /* SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_ */
