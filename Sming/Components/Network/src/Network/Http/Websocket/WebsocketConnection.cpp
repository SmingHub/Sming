/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebsocketConnection.cpp
 *
 ****/

#include "WebsocketConnection.h"
#include <BitManipulations.h>
#include <Crypto/Sha1.h>
#include <Data/WebHelpers/base64.h>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/XorOutputStream.h>
#include <Data/Stream/SharedMemoryStream.h>
#include <memory>

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

	ws_parser_init(&parser);
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
	auto hash = Crypto::Sha1().calculate(token);
	response.code = HTTP_STATUS_SWITCHING_PROTOCOLS;
	response.headers[HTTP_HEADER_CONNECTION] = WSSTR_UPGRADE;
	response.headers[HTTP_HEADER_UPGRADE] = WSSTR_WEBSOCKET;
	response.headers[HTTP_HEADER_SEC_WEBSOCKET_ACCEPT] = base64_encode(hash.data(), hash.size());

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
	int rc = ws_parser_execute(&parser, &parserSettings, this, at, size);
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

	if(connection->controlFrame.type == WS_FRAME_PONG && connection->wsPong) {
		connection->wsPong(*connection);
	}
	return WS_OK;
}

bool WebsocketConnection::send(const char* message, size_t length, ws_frame_type_t type)
{
	auto stream = new MemoryDataStream();
	if(stream == nullptr) {
		debug_e("Unable to create memory buffer");
		return false;
	}

	size_t written = stream->write(message, length);
	if(written != length) {
		debug_e("Unable to store data in memory buffer");
		delete stream;
		return false;
	}

	return send(stream, type, isClientConnection);
}

bool WebsocketConnection::send(IDataSourceStream* source, ws_frame_type_t type, bool useMask, bool isFin)
{
	// Ensure source gets destroyed if we return prematurely
	std::unique_ptr<IDataSourceStream> sourceRef(source);

	if(source == nullptr) {
		return false;
	}

	if(connection == nullptr) {
		return false;
	}

	if(!activated) {
		debug_e("WS Connection is not activated yet!");
		return false;
	}

	int available = source->available();
	if(available < 0) {
		debug_e("Streams without known size are not supported");
		return false;
	}

	debug_d("Sending: %d bytes, Type: %d\n", available, type);

	size_t packetLength = 2;
	uint16_t lengthValue = available;

	// calculate message length ....
	if(available <= 125) {
		lengthValue = available;
	} else if(available < 65536) {
		lengthValue = 126;
		packetLength += 2;
	} else {
		lengthValue = 127;
		packetLength += 8;
	}

	if(useMask) {
		packetLength += 4; // we use mask with size 4 bytes
	}

	uint8_t packet[packetLength];
	memset(packet, 0, packetLength);

	int i = 0;
	// byte 0
	if(isFin) {
		packet[i] |= bit(7); // set Fin
	}
	packet[i++] |= (uint8_t)type; // set opcode
	// byte 1
	if(useMask) {
		packet[i] |= bit(7); // set mask
	}

	// length
	if(lengthValue < 126) {
		packet[i++] |= lengthValue;
	} else if(lengthValue == 126) {
		packet[i++] |= 126;
		packet[i++] = (available >> 8) & 0xFF;
		packet[i++] = available & 0xFF;
	} else if(lengthValue == 127) {
		packet[i++] |= 127;
		packet[i++] = 0;
		packet[i++] = 0;
		packet[i++] = 0;
		packet[i++] = 0;
		packet[i++] = (available >> 24) & 0xFF;
		packet[i++] = (available >> 16) & 0xFF;
		packet[i++] = (available >> 8) & 0xFF;
		packet[i++] = (available)&0xFF;
	}

	if(useMask) {
		uint8_t maskKey[4] = {0x00, 0x00, 0x00, 0x00};
		for(uint8_t x = 0; x < sizeof(maskKey); x++) {
			maskKey[x] = (char)os_random();
			packet[i++] = maskKey[x];
		}

		auto xorStream = new XorOutputStream(source, maskKey, sizeof(maskKey));
		if(xorStream == nullptr) {
			return false;
		}
		sourceRef.release();
		sourceRef.reset(xorStream);
	}

	// send the header
	if(!connection->send(reinterpret_cast<const char*>(packet), packetLength)) {
		return false;
	}

	// Pass stream to connection
	return connection->send(sourceRef.release());
}

void WebsocketConnection::broadcast(const char* message, size_t length, ws_frame_type_t type)
{
	char* copy = new char[length];
	memcpy(copy, message, length);
	std::shared_ptr<const char> data(copy, [](const char* ptr) { delete[] ptr; });

	for(auto skt : websocketList) {
		auto stream = new SharedMemoryStream<const char>(data, length);
		skt->send(stream, type);
	}
}

void WebsocketConnection::close()
{
	debug_d("Terminating Websocket connection.");
	websocketList.removeElement(this);
	if(state != eWSCS_Closed) {
		state = eWSCS_Closed;
		if(isClientConnection) {
			uint16_t status = htons(1000);
			send(reinterpret_cast<char*>(&status), sizeof(status), WS_FRAME_CLOSE);
		}
		activated = false;
		if(wsDisconnect) {
			wsDisconnect(*this);
		}
	}

	if(connection) {
		connection->setTimeOut(2);
		connection = nullptr;
	}
}

void WebsocketConnection::reset()
{
	ws_parser_init(&parser);

	activated = false;
}
