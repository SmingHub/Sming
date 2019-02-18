/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebsocketConnection.cpp
 *
 ****/

#include "WebsocketConnection.h"

#include "../Services/WebHelpers/aw-sha1.h"
#include "../Services/WebHelpers/base64.h"

WebsocketList WebsocketConnection::websocketList;

WebsocketConnection::WebsocketConnection(HttpConnectionBase* connection, bool isClientConnection)
{
	setConnection(connection, isClientConnection);

	userData = (void*)this;

	memset(&parserSettings, 0, sizeof(parserSettings));
	parserSettings.on_data_begin = staticOnDataBegin;
	parserSettings.on_data_payload = staticOnDataPayload;
	parserSettings.on_data_end = staticOnDataEnd;
	parserSettings.on_control_begin = staticOnControlBegin;
	parserSettings.on_control_payload = staticOnControlPayload;
	parserSettings.on_control_end = staticOnControlEnd;

	ws_parser_init(&parser, &parserSettings);
	parser.user_data = (void*)this;
}

WebsocketConnection::~WebsocketConnection()
{
	state = eWSCS_Closed;
	close();
}

bool WebsocketConnection::bind(HttpRequest& request, HttpResponse& response)
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

	isClientConnection = false;

	return true;
}

bool WebsocketConnection::onConnected()
{
	activate();
	if(!websocketList.contains(this)) {
		websocketList.addElement(this);
	}

	if(wsConnect) {
		wsConnect(*this);
	}

	return true;
}

void WebsocketConnection::activate()
{
	activated = true;
	connection->setReceiveDelegate(TcpClientDataDelegate(&WebsocketConnection::processFrame, this));
}

bool WebsocketConnection::processFrame(TcpClient& client, char* at, int size)
{
	int rc = ws_parser_execute(&parser, (char*)at, size);
	if(rc != WS_OK) {
		debug_e("WebsocketResource error: %d %s\n", rc, ws_parser_error(rc));
		return false;
	}

	return true;
}

int WebsocketConnection::staticOnDataBegin(void* userData, ws_frame_type_t type)
{
	WebsocketConnection* connection = static_cast<WebsocketConnection*>(userData);
	if(connection == nullptr) {
		return -1;
	}

	connection->frameType = type;

	debug_d("data_begin: %s\n", type == WS_FRAME_TEXT ? "text" : type == WS_FRAME_BINARY ? "binary" : "?");

	return WS_OK;
}

int WebsocketConnection::staticOnDataPayload(void* userData, const char* at, size_t length)
{
	WebsocketConnection* connection = static_cast<WebsocketConnection*>(userData);
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

int WebsocketConnection::staticOnDataEnd(void* userData)
{
	return WS_OK;
}

int WebsocketConnection::staticOnControlBegin(void* userData, ws_frame_type_t type)
{
	WebsocketConnection* connection = static_cast<WebsocketConnection*>(userData);
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

int WebsocketConnection::staticOnControlPayload(void* userData, const char* data, size_t length)
{
	WebsocketConnection* connection = static_cast<WebsocketConnection*>(userData);
	if(connection == nullptr) {
		return -1;
	}

	connection->controlFrame.payload = (char*)data;
	connection->controlFrame.payloadLegth = length;

	return WS_OK;
}

int WebsocketConnection::staticOnControlEnd(void* userData)
{
	WebsocketConnection* connection = static_cast<WebsocketConnection*>(userData);
	if(connection == nullptr) {
		return -1;
	}

	if(connection->controlFrame.type == WS_FRAME_PING) {
		connection->send((const char*)connection->controlFrame.payload, connection->controlFrame.payloadLegth,
						 WS_FRAME_PONG);
	}

	return WS_OK;
}

void WebsocketConnection::sendString(const String& message)
{
	send(message.c_str(), message.length(), WS_FRAME_TEXT);
}

void WebsocketConnection::sendBinary(const uint8_t* data, int size)
{
	send((char*)data, size, WS_FRAME_BINARY);
}

void WebsocketConnection::send(const char* message, int length, ws_frame_type_t type)
{
	debug_d("Sending: %s, Type: %d\n", message, type);
	if(connection == nullptr) {
		return;
	}

	if(!activated) {
		debug_e("WS Connection is not activated yet!");
		return;
	}

	int bufferLength = length + 1 + 4 + 4;
	char buffer[bufferLength];
	size_t outLength = encodeFrame(type, message, length, buffer, bufferLength, isClientConnection);
	if(outLength) {
		connection->send((const char*)buffer, outLength);
	}
}

void WebsocketConnection::broadcast(const char* message, int length, ws_frame_type_t type)
{
	for(unsigned i = 0; i < websocketList.count(); i++) {
		websocketList[i]->send(message, length, type);
	}
}

size_t WebsocketConnection::encodeFrame(ws_frame_type_t type, const char* inData, size_t inLength, char* outData,
										size_t outLength, bool useMask, bool isFin)
{
	if(inLength > 0xFFFF) {
		return 0; // we don't support big payloads yet
	}

	int headerLength = 2;
	uint8_t maskKey[4] = {0x00, 0x00, 0x00, 0x00};
	if(inLength > 125) {
		headerLength = 4;
	}
	if(useMask) {
		headerLength += 4; // if present, mask is 4 bytes in header before payload
	}

	if(headerLength + inLength > outLength) {
		// not enough memory to store the data
		return 0;
	}

	memset(outData, 0, headerLength); //set initial header state to be all zero

	int i = 0;
	// byte 0
	if(isFin) {
		outData[i] |= bit(7); // set Fin
	}
	outData[i++] |= (uint8_t)type; // set opcode

	// byte 1
	if(useMask) {
		outData[i] |= bit(7); // set mask
	}

	if(inLength < 126) {
		outData[i++] |= inLength;
	} else {
		outData[i++] |= 126;
		outData[i++] = ((inLength >> 8) & 0xFF);
		outData[i++] = (inLength & 0xFF);
	}

	if(useMask) {
		for(uint8_t x = 0; x < sizeof(maskKey); x++) {
			maskKey[x] = (char)os_random();
			outData[i++] = maskKey[x];
		}

		for(size_t x = 0; x < inLength; x++) {
			outData[i++] = (inData[x] ^ maskKey[x % 4]);
		}
	} else {
		memcpy(&outData[i], inData, inLength);
		i += inLength;
	}

	return i;
}

bool WebsocketConnection::operator==(const WebsocketConnection& rhs) const
{
	return (this == &rhs);
}

WebsocketList& WebsocketConnection::getActiveWebsockets()
{
	return websocketList;
}

void WebsocketConnection::close()
{
	debug_d("Terminating Websocket connection.");
	websocketList.removeElement(this);
	if(state != eWSCS_Closed) {
		state = eWSCS_Closed;
		if(isClientConnection) {
			send((const char*)NULL, 0, WS_FRAME_CLOSE);
		}
		activated = false;
		if(wsDisconnect) {
			wsDisconnect(*this);
		}
	}

	if(connection) {
		connection->setTimeOut(1);
		connection = nullptr;
	}
}

void WebsocketConnection::reset()
{
	ws_parser_init(&parser, &parserSettings);
	parser.user_data = (void*)this;

	activated = false;
}

void WebsocketConnection::setUserData(void* userData)
{
	this->userData = userData;
}

void* WebsocketConnection::getUserData()
{
	return userData;
}

void WebsocketConnection::setConnectionHandler(WebsocketDelegate handler)
{
	wsConnect = handler;
}

void WebsocketConnection::setMessageHandler(WebsocketMessageDelegate handler)
{
	wsMessage = handler;
}

void WebsocketConnection::setBinaryHandler(WebsocketBinaryDelegate handler)
{
	wsBinary = handler;
}

void WebsocketConnection::setDisconnectionHandler(WebsocketDelegate handler)
{
	wsDisconnect = handler;
}
