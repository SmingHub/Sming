/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "WebSocketConnection.h"
#include "../Services/WebHelpers/aw-sha1.h"
#include "../Services/WebHelpers/base64.h"
#include "Data/Stream/MemoryDataStream.h"

WebSocketsList WebSocketConnection::_websocketList;

static const ws_parser_callbacks_t _parserSettings PROGMEM = {
	WSPARSER_CALLBACK(WebSocketConnection, on_data_begin),
	WSPARSER_CALLBACK(WebSocketConnection, on_data_payload),
	WSPARSER_CALLBACK(WebSocketConnection, on_data_end),
	WSPARSER_CALLBACK(WebSocketConnection, on_control_begin),
	WSPARSER_CALLBACK(WebSocketConnection, on_control_payload),
	WSPARSER_CALLBACK(WebSocketConnection, on_control_end)};

WebSocketConnection::WebSocketConnection(HttpServerConnection& conn) : _connection(conn)
{
	// Disable disconnection on connection idle (no rx/tx)
	conn.setTimeOut(USHRT_MAX);
}

WebSocketConnection::~WebSocketConnection()
{
	_isOpen = false;
	_stream = nullptr;
	close();
}

bool WebSocketConnection::initialize(HttpRequest& request, HttpResponse& response)
{
	String version = request.headers[hhfn_SecWebSocketVersion];
	version.trim();
	if (version.toInt() != WEBSOCKET_VERSION)
		return false;

	_isOpen = true;
	String token = request.headers[hhfn_SecWebSocketKey];
	token.trim();
	token += WEBSOCKET_SECRET;
	unsigned char hash[SHA1_SIZE];
	sha1(hash, token.c_str(), token.length());
	response.code = HTTP_STATUS_SWITCHING_PROTOCOLS;
	response.headers[hhfn_Connection] = WEBSOCKET_UPGRADE;
	response.headers[hhfn_Upgrade] = WEBSOCKET_WEBSOCKET;
	response.headers[hhfn_SecWebSocketAccept] = base64_encode(hash, SHA1_SIZE);

	// Response owns the stream, we just reference it for the duration of this message
	auto mem = new EndlessMemoryStream();
	if (!response.sendDataStream(mem))
		return false;
	_stream = mem;
	response.headers.remove(hhfn_TransferEncoding);

	_connection.userData = this;

	ws_parser_init(&_parser, &_parserSettings);
	_parser.user_data = this;

	if (!_websocketList.contains(this))
		_websocketList.addElement(this);

	if (_wsConnect)
		_wsConnect(*this);

	return true;
}

int WebSocketConnection::processFrame(HttpServerConnection& connection, HttpRequest& request, char* at, int size)
{
	int rc = ws_parser_execute(&_parser, (char*)at, size);
	if (rc != WS_OK) {
		debug_e("WebSocketResource error: %d %s\n", rc, ws_parser_error(rc));
		return -1;
	}

	return 0;
}

int WebSocketConnection::on_data_begin(ws_frame_type_t type)
{
	_frameType = type;

	debug_d("WebSocketConnection::on_data_begin(%s {%d})",
			type == WS_FRAME_TEXT ? _F("text") : type == WS_FRAME_BINARY ? _F("binary") : "?", type);

	return WS_OK;
}

int WebSocketConnection::on_data_payload(const char* at, size_t length)
{
	if (_frameType == WS_FRAME_TEXT) {
		if (_wsMessage)
			_wsMessage(*this, String(at, length));
	}
	else if (_frameType == WS_FRAME_BINARY) {
		if (_wsBinary)
			_wsBinary(*this, (uint8_t*)at, length);
	}

	return WS_OK;
}

int WebSocketConnection::on_data_end()
{
	return WS_OK;
}

int WebSocketConnection::on_control_begin(ws_frame_type_t type)
{
	_controlFrame.type = type;
	_controlFrame.payload = nullptr;
	_controlFrame.payloadLegth = 0;

	if (type == WS_FRAME_CLOSE)
		close();

	return WS_OK;
}

int WebSocketConnection::on_control_payload(const char* data, size_t length)
{
	_controlFrame.payload = (char*)data;
	_controlFrame.payloadLegth = length;

	return WS_OK;
}

int WebSocketConnection::on_control_end()
{
	if (_controlFrame.type == WS_FRAME_PING)
		send((const char*)_controlFrame.payload, _controlFrame.payloadLegth, WS_PONG_FRAME);

	return WS_OK;
}

/* @todo
 *
 * 	Sending JSON currently involves:
 *
 * 	1. Creating the JSON object
 * 	2. Printing the object into a String
 * 	3. Passing the string here, where we make a header and buffer it, yet again, into another stream
 * 	4. TcpClient takes the stream and outputs it to the TCP (which has it's own buffers)
 *
 * 	Lots of buffers. We want to skip (2) entirely and use a JsonObjectStream. It would allow us
 * 	to create and send the header before passing the stream through. Same applies to sending streams
 * 	of other stuff. We could also efficiently break it up into frames so sending of larger
 * 	messages would be possible.
 *
 */
void WebSocketConnection::send(const char* message, int length, wsFrameType type /* = WS_TEXT_FRAME*/)
{
	if (!_stream)
		return;

	uint8_t frameHeader[16] = {0};
	size_t headSize = sizeof(frameHeader);
	wsMakeFrame(nullptr, length, frameHeader, &headSize, type);
	_stream->write(frameHeader, headSize);
	_stream->write((uint8_t*)message, (size_t)length);
}

/* @todo
 *
 *	This is a pretty crude way of broadcasting a message. The message should be encoded
 *	only once, then the same buffered data written to each socket in turn. That needs to
 *	be handled at the TcpConnection level of things.
 */
void WebSocketConnection::broadcast(const char* message, int length, wsFrameType type /* = WS_TEXT_FRAME*/)
{
	for (unsigned i = 0; i < _websocketList.count(); i++)
		_websocketList[i]->send(message, length, type);
}

void WebSocketConnection::close()
{
	_websocketList.removeElement(this);
	if (_isOpen) {
		_isOpen = false;
		send(nullptr, 0, WS_CLOSING_FRAME);
		_stream = nullptr;
		if (_wsDisconnect)
			_wsDisconnect(*this);
	}

	_connection.setTimeOut(1);
}
