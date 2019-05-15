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

#include "Network/WebHelpers/aw-sha1.h"
#include "Network/WebHelpers/base64.h"

DEFINE_FSTR(WSSTR_CONNECTION, "connection")
DEFINE_FSTR(WSSTR_UPGRADE, "upgrade")
DEFINE_FSTR(WSSTR_WEBSOCKET, "websocket")
DEFINE_FSTR(WSSTR_HOST, "host")
DEFINE_FSTR(WSSTR_ORIGIN, "origin")
DEFINE_FSTR(WSSTR_KEY, "Sec-WebSocket-Key")
DEFINE_FSTR(WSSTR_PROTOCOL, "Sec-WebSocket-Protocol")
DEFINE_FSTR(WSSTR_VERSION, "Sec-WebSocket-Version")
DEFINE_FSTR(WSSTR_SECRET, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")

WebsocketList WebsocketConnection::websocketList;

/** @brief ws_parser function table
 * 	@note stored in flash memory; as it is word-aligned it can be accessed directly
 */
const ws_parser_callbacks_t WebsocketConnection::parserSettings PROGMEM = {.on_data_begin = staticOnDataBegin,
																		   .on_data_payload = staticOnDataPayload,
																		   .on_data_end = staticOnDataEnd,
																		   .on_control_begin = staticOnControlBegin,
																		   .on_control_payload = staticOnControlPayload,
																		   .on_control_end = staticOnControlEnd};

/** @brief Boilerplate code for ws_parser callbacks
 *  @note Obtain connection object and check it
 */
#define GET_CONNECTION()                                                                                               \
	auto connection = static_cast<WebsocketConnection*>(userData);                                                     \
	if(connection == nullptr) {                                                                                        \
		return -1;                                                                                                     \
	}

WebsocketConnection::WebsocketConnection(HttpConnection* connection, bool isClientConnection)
{
	setConnection(connection, isClientConnection);

	ws_parser_init(&parser, &parserSettings);
	parser.user_data = this;
}

bool WebsocketConnection::bind(HttpRequest& request, HttpResponse& response)
{
	String version = request.headers[HTTP_HEADER_SEC_WEBSOCKET_VERSION];
	version.trim();
	if(version.toInt() != WEBSOCKET_VERSION) {
		return false;
	}

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
	int rc = ws_parser_execute(&parser, at, size);
	if(rc != WS_OK) {
		debug_e("WebsocketResource error: %d %s\n", rc, ws_parser_error(rc));
		return false;
	}

	return true;
}

int WebsocketConnection::staticOnDataBegin(void* userData, ws_frame_type_t type)
{
	GET_CONNECTION();

	connection->frameType = type;

	debug_d("data_begin: %s\n", type == WS_FRAME_TEXT ? _F("text") : type == WS_FRAME_BINARY ? _F("binary") : "?");

	return WS_OK;
}

int WebsocketConnection::staticOnDataPayload(void* userData, const char* at, size_t length)
{
	GET_CONNECTION();

	if(connection->frameType == WS_FRAME_TEXT && connection->wsMessage) {
		connection->wsMessage(*connection, String(at, length));
	} else if(connection->frameType == WS_FRAME_BINARY && connection->wsBinary) {
		connection->wsBinary(*connection, reinterpret_cast<uint8_t*>(const_cast<char*>(at)), length);
	}

	return WS_OK;
}

int WebsocketConnection::staticOnDataEnd(void* userData)
{
	return WS_OK;
}

int WebsocketConnection::staticOnControlBegin(void* userData, ws_frame_type_t type)
{
	GET_CONNECTION();

	connection->controlFrame = WsFrameInfo(type, nullptr, 0);

	if(type == WS_FRAME_CLOSE) {
		connection->close();
	}

	return WS_OK;
}

int WebsocketConnection::staticOnControlPayload(void* userData, const char* data, size_t length)
{
	GET_CONNECTION();

	connection->controlFrame.payload = const_cast<char*>(data);
	connection->controlFrame.payloadLength = length;

	return WS_OK;
}

int WebsocketConnection::staticOnControlEnd(void* userData)
{
	GET_CONNECTION();

	if(connection->controlFrame.type == WS_FRAME_PING) {
		connection->send(connection->controlFrame.payload, connection->controlFrame.payloadLength, WS_FRAME_PONG);
	}

	return WS_OK;
}

void WebsocketConnection::send(const char* message, size_t length, ws_frame_type_t type)
{
	debug_d("Sending: %s, Type: %d\n", message, type);
	if(connection == nullptr) {
		return;
	}

	if(!activated) {
		debug_e("WS Connection is not activated yet!");
		return;
	}

	auto bufferLength = length + 1 + 4 + 4;
	char buffer[bufferLength];
	size_t outLength = encodeFrame(type, message, length, buffer, bufferLength, isClientConnection);
	if(outLength != 0) {
		connection->send(buffer, outLength);
	}
}

void WebsocketConnection::broadcast(const char* message, size_t length, ws_frame_type_t type)
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

void WebsocketConnection::close()
{
	debug_d("Terminating Websocket connection.");
	websocketList.removeElement(this);
	if(state != eWSCS_Closed) {
		state = eWSCS_Closed;
		if(isClientConnection) {
			send(nullptr, 0, WS_FRAME_CLOSE);
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
	parser.user_data = this;

	activated = false;
}
