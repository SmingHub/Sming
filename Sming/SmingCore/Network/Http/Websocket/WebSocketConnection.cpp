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
	state = eWSCS_Closed;
	stream = nullptr;
	close();
}

bool WebSocketConnection::initialize(HttpRequest& request, HttpResponse& response)
{
	String version = request.headers[HTTP_HEADER_SEC_WEBSOCKET_VERSION];
	version.trim();
	if(version.toInt() != WEBSOCKET_VERSION)
		return false;

	state = eWSCS_Open;
	String token = request.headers[HTTP_HEADER_SEC_WEBSOCKET_KEY];
	token.trim();
	token += WSSTR_SECRET;
	unsigned char hash[SHA1_SIZE];
	sha1(hash, token.c_str(), token.length());
	response.code = HTTP_STATUS_SWITCHING_PROTOCOLS;
	response.headers[HTTP_HEADER_CONNECTION] = WSSTR_UPGRADE;
	response.headers[HTTP_HEADER_UPGRADE] = WSSTR_WEBSOCKET;
	response.headers[HTTP_HEADER_SEC_WEBSOCKET_ACCEPT] = base64_encode(hash, SHA1_SIZE);

	delete stream;
	stream = new EndlessMemoryStream();
	response.sendDataStream(stream);

	connection->userData = (void*)this;

	memset(&parserSettings, 0, sizeof(parserSettings));
	parserSettings.on_data_begin = staticOnDataBegin;
	parserSettings.on_data_payload = staticOnDataPayload;
	parserSettings.on_data_end = staticOnDataEnd;
	parserSettings.on_control_begin = staticOnControlBegin;
	parserSettings.on_control_payload = staticOnControlPayload;
	parserSettings.on_control_end = staticOnControlEnd;

	ws_parser_init(&parser, &parserSettings);
	parser.user_data = (void*)this;

	if(!websocketList.contains(this)) {
		websocketList.addElement(this);
	}

	if(wsConnect) {
		wsConnect(*this);
	}

	return true;
}

int WebSocketConnection::processFrame(HttpServerConnection& connection, HttpRequest& request, char* at, int size)
{
	int rc = ws_parser_execute(&parser, (char*)at, size);
	if(rc != WS_OK) {
		debug_e("WebSocketResource error: %d %s\n", rc, ws_parser_error(rc));
		return -1;
	}

	return 0;
}

int WebSocketConnection::staticOnDataBegin(void* userData, ws_frame_type_t type)
{
	WebSocketConnection* connection = (WebSocketConnection*)userData;
	if(connection == nullptr) {
		return -1;
	}

	connection->frameType = type;

	debug_d("data_begin: %s\n", type == WS_FRAME_TEXT ? "text" : type == WS_FRAME_BINARY ? "binary" : "?");

	return WS_OK;
}

int WebSocketConnection::staticOnDataPayload(void* userData, const char* at, size_t length)
{
	WebSocketConnection* connection = (WebSocketConnection*)userData;
	if(connection == nullptr) {
		return -1;
	}

	if(connection->frameType == WS_FRAME_TEXT && connection->wsMessage) {
		connection->wsMessage(*connection, String(at, length));
	} else if(connection->frameType == WS_FRAME_BINARY && connection->wsBinary) {
		connection->wsBinary(*connection, (uint8_t*)at, length);
	}

	return WS_OK;
}

int WebSocketConnection::staticOnDataEnd(void* userData)
{
	return WS_OK;
}

int WebSocketConnection::staticOnControlBegin(void* userData, ws_frame_type_t type)
{
	WebSocketConnection* connection = (WebSocketConnection*)userData;
	if(connection == nullptr) {
		return -1;
	}

	connection->controlFrame.type = type;
	connection->controlFrame.payload = nullptr;
	connection->controlFrame.payloadLegth = 0;

	if(type == WS_FRAME_CLOSE) {
		connection->close();
	}

	return WS_OK;
}

int WebSocketConnection::staticOnControlPayload(void* userData, const char* data, size_t length)
{
	WebSocketConnection* connection = (WebSocketConnection*)userData;
	if(connection == nullptr) {
		return -1;
	}

	connection->controlFrame.payload = (char*)data;
	connection->controlFrame.payloadLegth = length;

	return WS_OK;
}

int WebSocketConnection::staticOnControlEnd(void* userData)
{
	WebSocketConnection* connection = (WebSocketConnection*)userData;
	if(connection == nullptr) {
		return -1;
	}

	if(connection->controlFrame.type == WS_FRAME_PING) {
		connection->send((const char*)connection->controlFrame.payload, connection->controlFrame.payloadLegth,
						 WS_PONG_FRAME);
	}

	return WS_OK;
}

void WebSocketConnection::send(const char* message, int length, wsFrameType type /* = WS_TEXT_FRAME*/)
{
	if(stream == nullptr) {
		return;
	}

	uint8_t frameHeader[16] = {0};
	size_t headSize = sizeof(frameHeader);
	wsMakeFrame(nullptr, length, frameHeader, &headSize, type);
	stream->write((uint8_t*)frameHeader, (uint16_t)headSize);
	stream->write((uint8_t*)message, (uint16_t)length);
}

void WebSocketConnection::broadcast(const char* message, int length, wsFrameType type /* = WS_TEXT_FRAME*/)
{
	for(int i = 0; i < websocketList.count(); i++) {
		websocketList[i]->send(message, length, type);
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

bool WebSocketConnection::operator==(const WebSocketConnection& rhs) const
{
	return (this == &rhs);
}

WebSocketsList& WebSocketConnection::getActiveWebSockets()
{
	return websocketList;
}

void WebSocketConnection::close()
{
	websocketList.removeElement(this);
	if(state != eWSCS_Closed) {
		state = eWSCS_Closed;
		send(nullptr, 0, WS_CLOSING_FRAME);
		stream = nullptr;
		if(wsDisconnect) {
			wsDisconnect(*this);
		}
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
