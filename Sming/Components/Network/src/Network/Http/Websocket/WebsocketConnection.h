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

/** @defgroup   websocket Websocket connection
 *  @brief      Provides websocket connection (server and client)
 *  @ingroup    http
 *  @{
 */

#define WEBSOCKET_VERSION 13 // 1.3

DECLARE_FSTR(WSSTR_UPGRADE)
DECLARE_FSTR(WSSTR_WEBSOCKET)
DECLARE_FSTR(WSSTR_SECRET)

class WebsocketConnection;

using WebsocketList = Vector<WebsocketConnection*>;

using WebsocketDelegate = Delegate<void(WebsocketConnection&)>;
using WebsocketMessageDelegate = Delegate<void(WebsocketConnection&, const String&)>;
using WebsocketBinaryDelegate = Delegate<void(WebsocketConnection&, uint8_t* data, size_t size)>;

/**
 * @brief Current state of Websocket connection
 */
enum WsConnectionState {
	eWSCS_Ready,
	eWSCS_Open,
	eWSCS_Closed,
};

struct WsFrameInfo {
	ws_frame_type_t type = WS_FRAME_TEXT;
	char* payload = nullptr;
	size_t payloadLength = 0;
};

class WebsocketConnection
{
public:
	/**
	 * @brief Constructs a websocket connection on top of http client or server connection
	 * @param connection the transport connection
	 * @param isClientConnection true when the passed connection is an http client connection
	 */
	WebsocketConnection(HttpConnection* connection = nullptr, bool isClientConnection = true);

	virtual ~WebsocketConnection()
	{
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
	bool send(const char* message, size_t length, ws_frame_type_t type = WS_FRAME_TEXT);

	/**
	 * @brief Sends websocket message from a String
	 * @param message String
	 * @param type
	 * @note A String may contain arbitrary data, not just text, so can use this for any frame type
	 */
	bool send(const String& message, ws_frame_type_t type = WS_FRAME_TEXT)
	{
		return send(message.c_str(), message.length(), type);
	}

	/**
	 * @brief Sends websocket message from a stream
	 * @param source The stream to send - we get ownership of the stream
	 * @param type
	 * @param useMask MUST be true for client connections
	 * @param isFin true if this is the final frame
	 *
	 * @retval bool true on success
	 */
	bool send(IDataSourceStream* source, ws_frame_type_t type = WS_FRAME_TEXT, bool useMask = false, bool isFin = true);

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
	bool sendString(const String& message)
	{
		return send(message, WS_FRAME_TEXT);
	}

	/**
	 * @brief Sends a binary websocket message
	 * @param data
	 * @param length
	 */
	bool sendBinary(const uint8_t* data, size_t length)
	{
		return send(reinterpret_cast<const char*>(data), length, WS_FRAME_BINARY);
	}

	/**
	 * @brief Closes a websocket connection (without closing the underlying http connection)
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
	 * @brief Sets the callback handler to be called when pong reply received
	 * @param handler
	 */
	void setPongHandler(WebsocketDelegate handler)
	{
		wsPong = handler;
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
	 * @param isClientConnection true when the passed connection is an http client connection
	 */
	void setConnection(HttpConnection* connection, bool isClientConnection = true);

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

protected:
	WebsocketDelegate wsConnect;
	WebsocketMessageDelegate wsMessage;
	WebsocketBinaryDelegate wsBinary;
	WebsocketDelegate wsPong;
	WebsocketDelegate wsDisconnect;

	void* userData = nullptr;

	WsConnectionState state;

private:
	ws_frame_type_t frameType = WS_FRAME_TEXT;
	WsFrameInfo controlFrame;

	ws_parser_t parser;
	static const ws_parser_callbacks_t parserSettings;

	static WebsocketList websocketList;

	HttpConnection* connection = nullptr;
	bool isClientConnection;
	bool activated = false;
};

/** @} */
