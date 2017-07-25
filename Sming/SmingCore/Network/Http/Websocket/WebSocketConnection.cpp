/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "WebSocketConnection.h"
#include "../../Services/WebHelpers/aw-sha1.h"
#include "../../Services/WebHelpers/base64.h"

WebSocketsList WebSocketConnection::websocketList;

WebSocketConnection::WebSocketConnection(HttpServerConnection* conn)
{
	connection = conn;
}

WebSocketConnection::~WebSocketConnection()
{
	websocketList.removeElement(*this);
}

bool WebSocketConnection::initialize(HttpRequest& request, HttpResponse& response)
{
	String version = request.getHeader("Sec-WebSocket-Version");
	version.trim();
	if (version.toInt() != 13) // 1.3
		return false;

	state = eWSCS_Open;
	String hash = request.getHeader("Sec-WebSocket-Key");
	hash.trim();
	hash = hash + secret;
	unsigned char data[SHA1_SIZE];
	char secure[SHA1_SIZE * 4];
	sha1(data, hash.c_str(), hash.length());
	base64_encode(SHA1_SIZE, data, SHA1_SIZE * 4, secure);
	response.code = HTTP_STATUS_SWITCHING_PROTOCOLS;
	response.setHeader("Connection", "Upgrade");
	response.setHeader("Upgrade", "websocket");
	response.setHeader("Sec-WebSocket-Accept", secure);

	connection->userData = (void *)this;

	websocketList.addElement(*this);

	memset(&parserSettings, 0, sizeof(parserSettings));
	parserSettings.on_data_begin = staticOnDataBegin;
	parserSettings.on_data_payload = staticOnDataPayload;
	parserSettings.on_data_end = staticOnDataEnd;
	parserSettings.on_control_begin = staticOnControlBegin;
	parserSettings.on_control_payload = staticOnControlPayload;
	parserSettings.on_control_end = staticOnControlEnd;

	ws_parser_init(&parser, &parserSettings);
	parser.user_data = (void*)this;

	if(wsConnect) {
		wsConnect(*this);
	}

	return true;
}

int WebSocketConnection::processFrame(HttpServerConnection& connection, HttpRequest& request, char *at, int size)
{
	int rc = ws_parser_execute(&parser, (char *)at, size);
	if (rc != WS_OK) {
		debugf("WebSocketResource error: %d %s\n", rc, ws_parser_error(rc));
		return -1;
	}

	return 0;
}

int WebSocketConnection::staticOnDataBegin(void* userData, ws_frame_type_t type) {
	WebSocketConnection *connection = (WebSocketConnection *)userData;
	if (connection == NULL) {
		return -1;
	}

	connection->frameType = type;

	debugf("data_begin: %s\n",
			type == WS_FRAME_TEXT ? "text" :
			type == WS_FRAME_BINARY ? "binary" :
			"?");

	return WS_OK;
}

int WebSocketConnection::staticOnDataPayload(void* userData, const char *at, size_t length) {
	WebSocketConnection *connection = (WebSocketConnection *)userData;
	if (connection == NULL) {
		return -1;
	}

	if (connection->frameType == WS_FRAME_TEXT && connection->wsMessage) {
		connection->wsMessage(*connection, String(at, length));
	} else if (connection->frameType == WS_FRAME_BINARY && connection->wsBinary) {
		connection->wsBinary(*connection, (uint8_t *) at, length);
	}

	return WS_OK;
}

int WebSocketConnection::staticOnDataEnd(void* userData)
{
	return WS_OK;
}

int WebSocketConnection::staticOnControlBegin(void* userData, ws_frame_type_t type)
{
	WebSocketConnection *connection = (WebSocketConnection *)userData;
	if (connection == NULL) {
		return -1;
	}

	connection->controlFrameType = type;

	if (type == WS_FRAME_CLOSE) {
		if(connection->wsDisconnect) {
			connection->wsDisconnect(*connection);
		}
		connection->close();
	}

	return WS_OK;
}

int WebSocketConnection::staticOnControlPayload(void* userData, const char *data, size_t length)
{
	return WS_OK;
}

int WebSocketConnection::staticOnControlEnd(void* userData)
{
	WebSocketConnection *connection = (WebSocketConnection *)userData;
	if (connection == NULL) {
		return -1;
	}

	if(connection->controlFrameType == WS_FRAME_PING) {
		// TODO: add control frame payload processing...
		connection->send((const char* )NULL, 0, WS_PONG_FRAME);
	}

	return WS_OK;
}


void WebSocketConnection::send(const char* message, int length, wsFrameType type /* = WS_TEXT_FRAME*/)
{
	uint8_t frameHeader[16] = {0};
	size_t headSize = sizeof(frameHeader);
	wsMakeFrame(nullptr, length, frameHeader, &headSize, type);
	connection->send((const char* )frameHeader, (uint16_t )headSize);
	if(length > 0) {
		connection->send((const char* )message, (uint16_t )length);
	}
}

void WebSocketConnection::broadcast(const char* message, int length, wsFrameType type /* = WS_TEXT_FRAME*/)
{
	for (int i = 0; i < websocketList.count(); i++) {
		websocketList[i].send(message, length, type);
	}
}

void WebSocketConnection::sendString(const String& message)
{
	send(message.c_str(), message.length());
}

void WebSocketConnection::sendBinary(const uint8_t* data, int size)
{
	send((char*)data, size, WS_BINARY_FRAME);
}

bool  WebSocketConnection::operator==(const WebSocketConnection &rhs) const
{
	return (this == &rhs);
}

WebSocketsList& WebSocketConnection::getActiveWebSockets()
{
	return websocketList;
}

void WebSocketConnection::close()
{
	websocketList.removeElement((const WebSocketConnection)*this);
	state = eWSCS_Closed;

	if(wsDisconnect) {
		wsDisconnect(*this);
	}

	connection->setTimeOut(1);
}

void WebSocketConnection::setUserData(void* userData)
{
	this->userData = userData;
}

void* WebSocketConnection::getUserData()
{
	return userData;
}


void WebSocketConnection::setConnectionHandler(WebSocketDelegate handler)
{
	wsConnect = handler;
}

void WebSocketConnection::setMessageHandler(WebSocketMessageDelegate handler)
{
	wsMessage = handler;
}

void WebSocketConnection::setBinaryHandler(WebSocketBinaryDelegate handler)
{
	wsBinary = handler;
}

void WebSocketConnection::setDisconnectionHandler(WebSocketDelegate handler)
{
	wsDisconnect = handler;
}
