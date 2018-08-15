/* 
 * File:   WebsocketClient.cpp
 * Original Author: https://github.com/hrsavla
 *
 * Created on August 4, 2015, 1:37 PM
 * This Websocket Client library is ported by hrsavla into Sming from
 * https://github.com/MORA99/Stokerbot/tree/master/Libraries/WebSocketClient 
 * 
 * Refactor and improve by https://github.com/avr39-ripe - Alexander V, Ribchansky
 * 
 */

#include "WebsocketClient.h"
#include "Data/HttpHeaders.h"
#include "Http/HttpRequest.h"


bool WebsocketClient::connect(const String& url, uint32_t sslOptions /* = 0 */)
{
	_uri = URL(url);
	bool useSsl = (_uri.protocol() == WEBSCOKET_SECURE_URL_PROTOCOL);
	if (!TcpClient::connect(_uri.host(), _uri.port(), useSsl, sslOptions))
		return false;

	debug_d("Connecting to Server");

	uint8_t keyStart[16];
	for (unsigned i = 0; i < sizeof(keyStart); ++i)
		keyStart[i] = 1 + os_random() % 255;
	_key = base64_encode(keyStart, sizeof(keyStart));

	// Server Connected / WS Upgrade request sent
	_mode = wsMode::Connecting;

	// Construct the HTTP connection request
	HttpRequest req(_uri);
	HttpHeaders headers;
	headers[hhfn_Upgrade] = WEBSOCKET_WEBSOCKET;
	headers[hhfn_Connection] = WEBSOCKET_UPGRADE;
	headers[hhfn_Host] = _uri.host();
	headers[hhfn_SecWebSocketKey] = _key;
	headers[hhfn_SecWebSocketProtocol] = F("chat");
	headers[hhfn_SecWebSocketVersion] = String(WEBSOCKET_VERSION);

	sendString(req.getRequestLine());
	for (unsigned i = 0; i < req.headers.count(); ++i)
		sendString(req.headers[i]);

	return true;
}

void WebsocketClient::onError(err_t err)
{
	if ((err == ERR_ABRT) || (err == ERR_RST))
		debug_e("TCP Connection Reseted or Aborted", err);
	else
		debug_e("Error  %d Occurred ", err);
	TcpClient::onError(err);
}

bool WebsocketClient::_verifyKey(char* buf, int size)
{
	String wsa = HttpHeaders::toString(hhfn_SecWebSocketAccept) + ": ";
	const char* serverHashedKey = strstri(buf, wsa.c_str());
	if (!serverHashedKey) {
		debug_e("wscli cannot find key");
		return false;
	}

	serverHashedKey += wsa.length();
	char* endKey = (char*)strstr(serverHashedKey, "\r\n");

	if (!endKey || endKey - buf > size) {
		debug_e("wscli cannot find key reason: %s", endKey ? "out of bounds" : "nullptr");
		return false;
	}

	*endKey = '\0';

	String keyToHash = _key + WEBSOCKET_SECRET;
	unsigned char hash[SHA1_SIZE];
	sha1(hash, keyToHash.c_str(), keyToHash.length());
	String b64hash = base64_encode(hash, sizeof(hash));
	if (b64hash != serverHashedKey) {
		debug_e("wscli key mismatch: %s | %s", serverHashedKey, b64hash.c_str());
		return false;
	}

	return true;
}

void WebsocketClient::onFinished(TcpClientState finishState)
{
	_mode = wsMode::Disconnected;
	uint8_t failed = (finishState == eTCS_Failed);
	if (failed)
		debug_e("Tcp Client failure...");
	else
		debug_d("Websocket Closed Normally.");

	if (_wsDisconnect)
		_wsDisconnect(*this, !failed);

	TcpClient::onFinished(finishState);
}

void WebsocketClient::disconnect()
{
	debug_d("Terminating Websocket connection.");
	_mode = wsMode::Disconnected;
	// Should send 0x87, 0x00 to server to tell it that I'm quitting here.
	uint8_t buf[2] = {0x87, 0x00};
	send((char*)buf, 2, true);
}

void WebsocketClient::_sendFrame(WSFrameType frameType, uint8_t* msg, uint16_t length)
{
	WebsocketFrameClass wsFrame;

	if (wsFrame.encodeFrame(frameType, msg, length, true, true, true)) {
		if (wsFrame.header())
			send((const char*)wsFrame.header(), wsFrame.headerLength(), false);
		send((const char*)wsFrame.payload(), wsFrame.payloadLength(), false);
	}
}

bool WebsocketClient::sendControlFrame(WSFrameType frameType, const String& payload)
{
	if (payload.length() > 127) {
		debug_w("Maximum length of payload is 127 bytes");
		return false;
	}

	uint32_t mask = 0;
	int size = 2 + payload.length() + 4 * mask;
	uint8_t* buf = new uint8_t[size];

	// if we have payload, generate random mask for it
	if (payload.length())
		mask = os_random();

	int pos = 0;
	buf[pos++] = (uint8_t)frameType;
	buf[pos++] = 0x00;
	buf[pos] |= bit(7);
	if (payload.length())
		buf[pos] += payload.length();
	++pos;

	buf[pos++] = (mask >> 24) & 0xFF;
	buf[pos++] = (mask >> 16) & 0xFF;
	buf[pos++] = (mask >> 8) & 0xFF;
	buf[pos++] = (mask >> 0) & 0xFF;

	WebsocketFrameClass::mask(payload, mask, (char*)&buf[pos]);

	send((char*)buf, size, false);

	return true;
}

err_t WebsocketClient::onReceive(pbuf* buf)
{
	if (!buf) {
		// Disconnected, close it
		return TcpClient::onReceive(buf);
	}

	uint16_t size = buf->tot_len;
	uint8_t* data = new uint8_t[size];

	pbuf_copy_partial(buf, data, size, 0);

	if (_mode == wsMode::Connecting) {
		if (_verifyKey((char*)data, size)) {
			_mode = wsMode::Connected;
			//   debug_d("Key Verified. Websocket Handshake completed");
			sendPing();
		}
		else {
			// Handshake was not proper.
			_mode = wsMode::Disconnected;
		}

		if (_wsConnect)
			_wsConnect(*this, _mode);
	}
	else if (_mode == wsMode::Connected) {
		WebsocketFrameClass wsFrame;
		do {
			if (wsFrame.decodeFrame(data, size)) {
				switch (wsFrame.frameType()) {
				case WSFrameType::text:
					//						debug_d("Got text frame");
					if (_wsMessage)
						_wsMessage(*this, wsFrame.payloadString());
					break;

				case WSFrameType::binary:
					//						debug_d("Got binary frame");
					if (_wsBinary)
						_wsBinary(*this, wsFrame.payload(), wsFrame.payloadLength());
					break;

				case WSFrameType::close:
					debug_d("Got Disconnect request from server.\n");
					//RFC requires we return a close op code before closing the connection
					disconnect();
					break;

				case WSFrameType::ping:
					debug_d("Got ping ...");
					//Need to send Pong in response to Ping
					sendPong(wsFrame.payloadString());
					break;

				case WSFrameType::pong:
					debug_d("Got pong ...");
					//A pong can contain app data, but shouldnt if we didnt send any...
					break;

				case WSFrameType::error:
					debug_e("ERROR parsing frame!");
					break;

				case WSFrameType::incomplete:
					debug_w("INCOMPLETE websocket frame!");
					break;

				default:
					debug_w("Unknown frameType: %d", wsFrame.frameType());
				}
			}
		} while (wsFrame.next());
	}

	delete[] data;
	return TcpClient::onReceive(buf);
}
