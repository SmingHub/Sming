/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/*
 * 17/7/2018
 *
 *
 */

#ifndef SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_
#define SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_

#include "Network/TcpServer.h"
#include "../HttpServerConnection.h"
#include "../Services/cWebsocket/websocket.h"
extern "C" {
#include "../ws_parser/ws_parser.h"
}

class WebSocketConnection;

typedef Vector<WebSocketConnection*> WebSocketsList;

typedef Delegate<void(WebSocketConnection&)> WebSocketDelegate;
typedef Delegate<void(WebSocketConnection&, const String&)> WebSocketMessageDelegate;
typedef Delegate<void(WebSocketConnection&, uint8_t* data, size_t size)> WebSocketBinaryDelegate;

typedef struct
{
	ws_frame_type_t type;
	char* payload;
	size_t payloadLegth;
} WsFrameInfo;

/*
 * Websocket parser uses static callback table (in structure).
 * Like the HTTP parser (in HttpCommon.h) these macros provide a way to implement
 * the boilerplate code.
 *
 *	@todo Consider implementing a wrapper class for the Websocket parser, or even
 *	refactoring the callbacks as a virtual abstract class; considering the function
 *	table is basically just a VMT this should be trivial.
 *	Note also that we're using both an external parser library plus the 'cWebsocket'
 *	service. These need to be better integrated to eliminate duplicate definitions, etc.
 */
#define WSPARSER_METHOD_0(_cls, _method)                                                                               \
public:                                                                                                                \
	static int static_##_method(void* userData)                                                                        \
	{                                                                                                                  \
		auto connection = (_cls*)userData;                                                                             \
		return connection ? connection->_method() : -1;                                                                \
	}                                                                                                                  \
                                                                                                                       \
private:                                                                                                               \
	int _method();

#define WSPARSER_METHOD_2(_cls, _method)                                                                               \
public:                                                                                                                \
	static int static_##_method(void* userData, const char* at, size_t length)                                         \
	{                                                                                                                  \
		auto connection = (_cls*)userData;                                                                             \
		return connection ? connection->_method(at, length) : -1;                                                      \
	}                                                                                                                  \
                                                                                                                       \
private:                                                                                                               \
	int _method(const char* at, size_t length);

#define WSPARSER_METHOD_1(_cls, _method)                                                                               \
public:                                                                                                                \
	static int static_##_method(void* userData, ws_frame_type_t type)                                                  \
	{                                                                                                                  \
		auto connection = (_cls*)userData;                                                                             \
		return connection ? connection->_method(type) : -1;                                                            \
	}                                                                                                                  \
                                                                                                                       \
private:                                                                                                               \
	int _method(ws_frame_type_t type);

// The method name for emitting parser settings table
#define WSPARSER_CALLBACK(_cls, _method)                                                                               \
	_method:                                                                                                           \
	_cls::static_##_method

class WebSocketConnection {
public:
	WebSocketConnection(HttpServerConnection& conn);
	virtual ~WebSocketConnection();

	bool initialize(HttpRequest& request, HttpResponse& response);

	virtual void send(const char* message, int length, wsFrameType type = WS_TEXT_FRAME);

	static void broadcast(const char* message, int length, wsFrameType type = WS_TEXT_FRAME);

	void sendString(const String& message)
	{
		send(message.c_str(), message.length());
	}

	void sendBinary(const uint8_t* data, int size)
	{
		send((char*)data, size, WS_BINARY_FRAME);
	}

	void close();

	HttpServerConnection& getHttpConnection()
	{
		return _connection;
	}

	void setUserData(void* userData)
	{
		_userData = userData;
	}

	void* getUserData()
	{
		return _userData;
	}

	// @deprecated
	bool operator==(const WebSocketConnection& rhs) const
	{
		return (this == &rhs);
	}
	// @end deprecated

	static WebSocketsList& getActiveWebSockets()
	{
		return _websocketList;
	}

	void setConnectionHandler(WebSocketDelegate handler)
	{
		_wsConnect = handler;
	}

	void setMessageHandler(WebSocketMessageDelegate handler)
	{
		_wsMessage = handler;
	}

	void setBinaryHandler(WebSocketBinaryDelegate handler)
	{
		_wsBinary = handler;
	}

	void setDisconnectionHandler(WebSocketDelegate handler)
	{
		_wsDisconnect = handler;
	}

	int processFrame(HttpServerConnection& connection, HttpRequest& request, char* at, int size);

protected:
	//	bool is(HttpServerConnection* conn)
	//	{
	//		return &_connection == conn;
	//	}

	WSPARSER_METHOD_1(WebSocketConnection, on_data_begin)
	WSPARSER_METHOD_2(WebSocketConnection, on_data_payload)
	WSPARSER_METHOD_0(WebSocketConnection, on_data_end)
	WSPARSER_METHOD_1(WebSocketConnection, on_control_begin)
	WSPARSER_METHOD_2(WebSocketConnection, on_control_payload)
	WSPARSER_METHOD_0(WebSocketConnection, on_control_end)

protected:
	WebSocketDelegate _wsConnect = nullptr;
	WebSocketMessageDelegate _wsMessage = nullptr;
	WebSocketBinaryDelegate _wsBinary = nullptr;
	WebSocketDelegate _wsDisconnect = nullptr;

private:
	bool _isOpen = false;

	void* _userData = nullptr;
	HttpServerConnection& _connection;

	ws_frame_type_t _frameType = WS_FRAME_TEXT;
	WsFrameInfo _controlFrame = { };

	ws_parser_t _parser;

	static WebSocketsList _websocketList;

	// Outgoing message content is stored here, but we don't own the stream
	ReadWriteStream* _stream = nullptr;
};

#endif /* SMINGCORE_NETWORK_WEBSOCKETCONNECTION_H_ */
