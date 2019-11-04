/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebsocketConnection.h
 *
 ****/

#pragma once

#include "Network/TcpServer.h"
#include "../HttpConnection.h"
extern "C" {
#include "ws_parser/ws_parser.h"
}

/** @defgroup   Websocket connection
 *  @brief      Provides websocket connection (server and client)
 *  @ingroup    websocket http
 *  @{
 */

#define WEBSOCKET_VERSION 13 // 1.3

DECLARE_FSTR(WSSTR_CONNECTION)
DECLARE_FSTR(WSSTR_UPGRADE)
DECLARE_FSTR(WSSTR_WEBSOCKET)
DECLARE_FSTR(WSSTR_HOST)
DECLARE_FSTR(WSSTR_ORIGIN)
DECLARE_FSTR(WSSTR_KEY)
DECLARE_FSTR(WSSTR_PROTOCOL)
DECLARE_FSTR(WSSTR_VERSION)
DECLARE_FSTR(WSSTR_SECRET)

class WebsocketConnection;

typedef Vector<WebsocketConnection*> WebsocketList;

typedef Delegate<void(WebsocketConnection&)> WebsocketDelegate;
typedef Delegate<void(WebsocketConnection&, const String&)> WebsocketMessageDelegate;
typedef Delegate<void(WebsocketConnection&, uint8_t* data, size_t size)> WebsocketBinaryDelegate;

enum WsConnectionState { eWSCS_Ready, eWSCS_Open, eWSCS_Closed };

struct WsFrameInfo {
	ws_frame_type_t type = WS_FRAME_TEXT;
	char* payload = nullptr;
	size_t payloadLength = 0;

	WsFrameInfo() = default;

	WsFrameInfo(ws_frame_type_t type, char* payload, size_t payloadLength)
		: type(type), payload(payload), payloadLength(payloadLength)
	{
	}
};

class WebsocketConnection
{
public:
	/**
	 * @brief Constructs a websocket connection on top of http client or server connection
	 * @param connection the transport connection
	 * @param isClientConnection true when the passed connection is an http client conneciton
	 */
	WebsocketConnection(HttpConnection* connection, bool isClientConnection = true);

	virtual ~WebsocketConnection()
	{
		state = eWSCS_Closed;
		close();
	}

	/**
	 * @brief Binds websocket connection to an http server connection
	 * @param request
	 * @param response
	 * @retval bool true on success, false otherwise
	 */
	bool bind(HttpRequest& request, HttpResponse& response);

	/**
	 * @brief Sends a websocket message from a buffer
	 * @param message
	 * @param length Quantity of data in message
	 * @param type
	 */
	virtual void send(const char* message, size_t length, ws_frame_type_t type = WS_FRAME_TEXT);

	/**
	 * @brief Sends websocket message from a String
	 * @param message String
	 * @param type
	 * @note A String may contain arbitrary data, not just text, so can use this for any frame type
	 */
	void send(const String& message, ws_frame_type_t type = WS_FRAME_TEXT)
	{
		send(message.c_str(), message.length(), type);
	}

	/**
	 * @brief Broadcasts a message to all active websocket connections
	 * @param message
	 * @param length
	 * @param type
	 */
	static void broadcast(const char* message, size_t length, ws_frame_type_t type = WS_FRAME_TEXT);

	/**
	 * @brief Broadcasts a message to all active websocket connections
	 * @param message
	 * @param type
	 */
	static void broadcast(const String& message, ws_frame_type_t type = WS_FRAME_TEXT)
	{
		broadcast(message.c_str(), message.length(), type);
	}

	/**
	 * @brief Sends a string websocket message
	 * @param message
	 */
	void sendString(const String& message)
	{
		send(message, WS_FRAME_TEXT);
	}

	/**
	 * @brief Sends a binary websocket message
	 * @param data
	 * @param length
	 */
	void sendBinary(const uint8_t* data, size_t length)
	{
		send(reinterpret_cast<const char*>(data), length, WS_FRAME_BINARY);
	}

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
	 * @param userData
	 */
	void setUserData(void* userData)
	{
		this->userData = userData;
	}

	/**
	 * @brief Retrieves user data attached
	 * @retval void* The user data previously set by `setUserData()`
	 */
	void* getUserData()
	{
		return userData;
	}

	/** @brief	Test if another connection refers to the same object
	 *  @param	rhs The other WebsocketConnection to compare with
	 *  @retval	bool
	 */
	bool operator==(const WebsocketConnection& rhs) const
	{
		return (this == &rhs);
	}

	/**
	 * @brief Obtain the list of active websockets
	 * @retval const WebsocketList&
	 * @note Return value is const as only restricted operations should be carried out on the list.
	 */
	static const WebsocketList& getActiveWebsockets()
	{
		return websocketList;
	}

	/**
	 * @brief Sets the callback handler to be called after successful websocket connection
	 * @param handler
	 */
	void setConnectionHandler(WebsocketDelegate handler)
	{
		wsConnect = handler;
	}

	/**
	 * @brief Sets the callback handler to be called after a websocket message is received
	 * @param handler
	 */
	void setMessageHandler(WebsocketMessageDelegate handler)
	{
		wsMessage = handler;
	}

	/**
	 * @brief Sets the callback handler to be called after a binary websocket message is received
	 * @param handler
	 */
	void setBinaryHandler(WebsocketBinaryDelegate handler)
	{
		wsBinary = handler;
	}

	/**
	 * @brief Sets the callback handler to be called before closing a websocket connection
	 * @param handler
	 */
	void setDisconnectionHandler(WebsocketDelegate handler)
	{
		wsDisconnect = handler;
	}

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
	 * @retval HttpConnection*
	 */
	HttpConnection* getConnection()
	{
		return connection;
	}

	/**
	 * @brief Sets the underlying (transport ) HTTP connection
	 * @param connection the transport connection
	 * @param isClientConnection true when the passed connection is an http client conneciton
	 */
	void setConnection(HttpConnection* connection, bool isClientConnection = true)
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
	// Static handlers for ws_parser
	static int staticOnDataBegin(void* userData, ws_frame_type_t type);
	static int staticOnDataPayload(void* userData, const char* at, size_t length);
	static int staticOnDataEnd(void* userData);
	static int staticOnControlBegin(void* userData, ws_frame_type_t type);
	static int staticOnControlPayload(void* userData, const char*, size_t length);
	static int staticOnControlEnd(void* userData);

	/** @brief Callback handler to process a received TCP data frame
	 *  @param client
	 *  @param at
	 *  @param size
	 *  @retval bool true if data parsing successful
	 */
	bool processFrame(TcpClient& client, char* at, int size);

	/** @brief Encode user content into a valid websocket frame
	 *  @param type
	 *  @param inData
	 *  @param inLength
	 *  @param outData
	 *  @param outLength
	 *  @param useMask MUST be true for client connections
	 *  @param isFin true if this is the final frame
	 *  @retval size_t Size of encoded frame
	 */
	size_t encodeFrame(ws_frame_type_t type, const char* inData, size_t inLength, char* outData, size_t outLength,
					   bool useMask = true, bool isFin = true);

protected:
	WebsocketDelegate wsConnect = nullptr;
	WebsocketMessageDelegate wsMessage = nullptr;
	WebsocketBinaryDelegate wsBinary = nullptr;
	WebsocketDelegate wsDisconnect = nullptr;

	void* userData = nullptr;

	WsConnectionState state = eWSCS_Ready;

private:
	ws_frame_type_t frameType = WS_FRAME_TEXT;
	WsFrameInfo controlFrame;

	ws_parser_t parser;
	static const ws_parser_callbacks_t parserSettings;

	static WebsocketList websocketList;

	bool isClientConnection = true;

	HttpConnection* connection = nullptr;
	bool activated = false;
};

/** @} */
