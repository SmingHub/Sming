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
#include <Crypto/Sha1.h>
#include <Data/WebHelpers/base64.h>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/XorOutputStream.h>
#include <Data/Stream/SharedMemoryStream.h>

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
const ws_parser_callbacks_t WebsocketConnection::parserSettings PROGMEM{
	.on_data_begin = staticOnDataBegin,
	.on_data_payload = staticOnDataPayload,
	.on_data_end = staticOnDataEnd,
	.on_control_begin = staticOnControlBegin,
	.on_control_payload = staticOnControlPayload,
	.on_control_end = staticOnControlEnd,
};

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

void WebsocketConnection::setConnection(HttpConnection* connection, bool isClientConnection)
{
	assert(this->connection == nullptr);
	this->connection = connection;
	this->isClientConnection = isClientConnection;
	this->state = connection ? eWSCS_Ready : eWSCS_Closed;
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

	switch(connection->frameType) {
	case WS_FRAME_TEXT:
		if(connection->wsMessage) {
			connection->wsMessage(*connection, String(at, length));
		}
		break;
	case WS_FRAME_BINARY:
		if(connection->wsBinary) {
			connection->wsBinary(*connection, reinterpret_cast<uint8_t*>(const_cast<char*>(at)), length);
		}
		break;
	case WS_FRAME_CLOSE:
	case WS_FRAME_PING:
	case WS_FRAME_PONG:
		break;
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

	connection->controlFrame = WsFrameInfo{type};

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

	switch(connection->controlFrame.type) {
	case WS_FRAME_PING:
		connection->send(connection->controlFrame.payload, connection->controlFrame.payloadLength, WS_FRAME_PONG);
		break;
	case WS_FRAME_PONG:
		if(connection->wsPong) {
			connection->wsPong(*connection);
		}
		break;

	case WS_FRAME_CLOSE:
		debug_hex(DBG, "WS: CLOSE", connection->controlFrame.payload, connection->controlFrame.payloadLength);
		connection->close();
		break;

	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:
		break;
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
		debug_w("WS: No source");
		return false;
	}

	if(connection == nullptr) {
		debug_w("WS: No connection");
		return false;
	}

	if(!activated) {
		debug_e("WS: Not activated");
		return false;
	}

	int available = source->available();
	if(available < 0) {
		debug_e("WS: Unknown stream size");
		return false;
	}

	debug_d("WS: Sending %d bytes, type %d", available, type);

	// Construct packet
	uint8_t packet[16]{};
	unsigned len = 0;
	if(isFin) {
		packet[len] |= _BV(7); // set Fin
	}
	packet[len++] |= type; // set opcode
	if(useMask) {
		packet[len] |= _BV(7); // set mask
	}
	// length
	if(available <= 125) {
		packet[len++] |= available;
	} else if(available <= 0xffff) {
		packet[len++] |= 126;
		packet[len++] = available >> 8;
		packet[len++] = available;
	} else {
		packet[len++] |= 127;
		len += 4; // All 0
		packet[len++] = available >> 24;
		packet[len++] = available >> 16;
		packet[len++] = available >> 8;
		packet[len++] = available;
	}
	if(useMask) {
		uint8_t maskKey[4];
		os_get_random(maskKey, sizeof(maskKey));
		memcpy(&packet[len], maskKey, sizeof(maskKey));
		len += sizeof(maskKey);

		auto xorStream = new XorOutputStream(source, maskKey, sizeof(maskKey));
		if(xorStream == nullptr) {
			return false;
		}
		sourceRef.release();
		sourceRef.reset(xorStream);
	}

	// send the header
	if(!connection->send(reinterpret_cast<const char*>(packet), len)) {
		return false;
	}

	// Pass stream to connection
	if(!connection->send(sourceRef.release())) {
		return false;
	}
	connection->commit();
	return true;
}

void WebsocketConnection::broadcast(const char* message, size_t length, ws_frame_type_t type)
{
	std::shared_ptr<char[]> data(new char[length]);
	if(!data) {
		return;
	}
	memcpy(data.get(), message, length);

	for(auto skt : websocketList) {
		auto stream = new SharedMemoryStream<const char[]>(data, length);
		skt->send(stream, type);
	}
}

void WebsocketConnection::close()
{
	if(connection == nullptr) {
		return;
	}

	debug_d("WS: Terminating connection %p, state %u", connection, state);
	websocketList.removeElement(this);
	if(state != eWSCS_Closed) {
		state = eWSCS_Closed;
		if(controlFrame.type == WS_FRAME_CLOSE) {
			send(controlFrame.payload, controlFrame.payloadLength, WS_FRAME_CLOSE);
		} else {
			uint16_t status = htons(1000);
			send(reinterpret_cast<char*>(&status), sizeof(status), WS_FRAME_CLOSE);
		}
		activated = false;
		if(wsDisconnect) {
			wsDisconnect(*this);
		}
	}

	connection->setTimeOut(2);
	connection->setAutoSelfDestruct(true);
	connection = nullptr;
}

void WebsocketConnection::reset()
{
	ws_parser_init(&parser);

	activated = false;
}
