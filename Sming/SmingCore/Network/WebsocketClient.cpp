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
#include "../Services/cWebsocket/websocket.h"
#include "Network/Http/HttpHeaders.h"

void WebsocketClient::setWebSocketMessageHandler(WebSocketClientMessageDelegate handler)
{
	wsMessage = handler;
}

void WebsocketClient::setWebSocketBinaryHandler(WebSocketClientBinaryDelegate handler)
{
	wsBinary = handler;
}

void WebsocketClient::setWebSocketDisconnectedHandler(WebSocketClientDisconnectDelegate handler)
{
	wsDisconnect = handler;
}

void WebsocketClient::setWebSocketConnectedHandler(WebSocketClientConnectedDelegate handler)
{
	wsConnect = handler;
}

bool WebsocketClient::connect(String url, uint32_t sslOptions /* = 0 */)
{
	_uri = URL(url);
	_url = url;
	bool useSsl = false;
	if(_uri.Protocol == WEBSOCKET_SECURE_URL_PROTOCOL) {
		useSsl = true;
	}
	TcpClient::connect(_uri.Host, _uri.Port, useSsl, sslOptions);
	debug_d("Connecting to Server");

	_mode = wsMode::Connecting; // Server Connected / WS Upgrade request sent

	uint8_t keyStart[16];
	for(unsigned i = 0; i < sizeof(keyStart); ++i) {
		keyStart[i] = 1 + os_random() % 255;
	}
	_key = base64_encode(keyStart, sizeof(keyStart));

	sendString(F("GET "));
	if(_uri.Path != "") {
		sendString(_uri.Path);
	} else {
		sendString("/");
	}
	sendString(F(" HTTP/1.1\r\n"));
	HttpHeaders headers;
	sendString(headers.toString(HTTP_HEADER_UPGRADE, WSSTR_WEBSOCKET));
	sendString(headers.toString(HTTP_HEADER_CONNECTION, WSSTR_UPGRADE));
	sendString(headers.toString(HTTP_HEADER_HOST, _uri.Host));
	sendString(headers.toString(HTTP_HEADER_SEC_WEBSOCKET_KEY, _key));
	sendString(headers.toString(HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL, F("chat")));
	sendString(headers.toString(HTTP_HEADER_SEC_WEBSOCKET_VERSION, String(WEBSOCKET_VERSION)));
	sendString("\r\n", false);
	return true;
}

void WebsocketClient::onError(err_t err)
{
	if((err == ERR_ABRT) || (err == ERR_RST)) {
		debug_e("TCP Connection Reseted or Aborted", err);
	} else {
		debug_e("Error  %d Occurred ", err);
	}
	TcpClient::onError(err);
}

bool WebsocketClient::_verifyKey(char* buf, int size)
{
	HttpHeaders headers;
	String wsa = headers.toString(HTTP_HEADER_SEC_WEBSOCKET_ACCEPT, String::nullstr);
	const char* serverHashedKey = strstri(buf, wsa.c_str());

	if(!serverHashedKey) {
		debug_e("wscli cannot find key");
		return false;
	}

	serverHashedKey += wsa.length();
	char* endKey = strstr(serverHashedKey, "\r\n");

	if(!endKey || endKey - buf > size) {
		debug_e("wscli cannot find key reason: %s", endKey ? _F("out of bounds") : _F("NULL"));
		return false;
	}

	*endKey = '\0';

	String keyToHash = _key + WSSTR_SECRET;
	unsigned char hash[SHA1_SIZE];
	sha1(hash, keyToHash.c_str(), keyToHash.length());
	String base64hash = base64_encode(hash, sizeof(hash));
	if(base64hash != serverHashedKey) {
		debug_e("wscli key mismatch: %s | %s", serverHashedKey, base64hash.c_str());
		return false;
	}

	return true;
}

void WebsocketClient::onFinished(TcpClientState finishState)
{
	_mode = wsMode::Disconnected;
	uint8_t failed = (finishState == eTCS_Failed);
	if(failed) {
		debug_e("Tcp Client failure...");
	} else {
		debug_d("Websocket Closed Normally.");
	}

	if(wsDisconnect) {
		wsDisconnect(*this, !failed);
	}
	TcpClient::onFinished(finishState);
}

bool WebsocketClient::sendPing(const String& payload /* = "" */)
{
	debug_d("Sending PING");
	return sendControlFrame(WSFrameType::ping, payload);
}

bool WebsocketClient::sendPong(const String& payload /* = "" */)
{
	debug_d("Sending PONG");
	return sendControlFrame(WSFrameType::pong, payload);
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
	uint8_t result = wsFrame.encodeFrame(frameType, msg, length, true, true, true);

	if(result && wsFrame._header == nullptr) {
		send((char*)&wsFrame._payload[0], wsFrame._payloadLength, false);
	} else if(result) {
		send((char*)&wsFrame._header[0], wsFrame._headerLength, false);
		send((char*)&wsFrame._payload[0], wsFrame._payloadLength, false);
	}
}
void WebsocketClient::sendBinary(uint8_t* msg, uint16_t length)
{
	_sendFrame(WSFrameType::binary, msg, length);
}

void WebsocketClient::sendMessage(char* msg, uint16_t length)
{
	_sendFrame(WSFrameType::text, (uint8_t*)msg, length);
}

void WebsocketClient::sendMessage(const String& str)
{
	_sendFrame(WSFrameType::text, (uint8_t*)str.c_str(), str.length() + 1);
}

bool WebsocketClient::sendControlFrame(WSFrameType frameType, const String& payload /* = "" */)
{
	if(payload.length() > 127) {
		debug_w("Maximum length of payload is 127 bytes");
		return false;
	}

	uint32_t mask = 0;
	int size = 2 + payload.length() + 4 * mask;
	uint8_t* buf = new uint8_t[size];

	// if we have payload, generate random mask for it
	if(payload.length()) {
		mask = ESP8266_DREG(0x20E44); // See: http://esp8266-re.foogod.com/wiki/Random_Number_Generator
	}

	int pos = 0;
	buf[pos++] = (uint8_t)frameType;
	buf[pos++] = 0x00;
	buf[pos] |= bit(7);

	if(payload.length()) {
		buf[pos] += payload.length();
	}

	buf[++pos] = (mask >> 24) & 0xFF;
	buf[++pos] = (mask >> 16) & 0xFF;
	buf[++pos] = (mask >> 8) & 0xFF;
	buf[++pos] = (mask >> 0) & 0xFF;

	WebsocketFrameClass::mask(payload, mask, (char*)(buf + pos + 1));

	send((char*)buf, size, false);

	return true;
}

err_t WebsocketClient::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		// Disconnected, close it
		return TcpClient::onReceive(buf);
	}

	uint16_t size = buf->tot_len;
	uint8_t* data = new uint8_t[size];

	pbuf_copy_partial(buf, data, size, 0);

	switch(_mode) {
	case wsMode::Connecting:
		if(_verifyKey((char*)data, size) == true) {
			_mode = wsMode::Connected;
			//   debug_d("Key Verified. Websocket Handshake completed");
			sendPing();
		} else {
			_mode = wsMode::Disconnected; // Handshake was not proper.
		}

		if(wsConnect) {
			wsConnect(*this, _mode);
		}
		break;

	case wsMode::Connected: {
		WebsocketFrameClass wsFrame;
		do {
			if(wsFrame.decodeFrame(data, size)) {
				switch(wsFrame._frameType) {
				case WSFrameType::text: {
					//						debug_d("Got text frame");
					String msg;
					msg.setString((char*)wsFrame._payload, wsFrame._payloadLength);
					if(wsMessage) {
						wsMessage(*this, msg.c_str());
					}
					break;
				}
				case WSFrameType::binary: {
					//						debug_d("Got binary frame");
					if(wsBinary) {
						wsBinary(*this, wsFrame._payload, wsFrame._payloadLength);
					}
					break;
				}
				case WSFrameType::close: {
					debug_d("Got Disconnect request from server.\n");
					//RFC requires we return a close op code before closing the connection
					disconnect();
					break;
				}
				case WSFrameType::ping: {
					debug_d("Got ping ...");
					sendPong(String((char*)wsFrame._payload,
									wsFrame._payloadLength)); //Need to send Pong in response to Ping
					break;
				}
				case WSFrameType::pong: {
					debug_d("Got pong ...");
					//A pong can contain app data, but shouldnt if we didnt send any...
					break;
				}
				case WSFrameType::error: {
					debug_e("ERROR parsing frame!");
					break;
				}
				case WSFrameType::incomplete: {
					debug_w("INCOMPLETE websocket frame!");
					break;
				}
				default: {
					debug_w("Unknown frameType: %d", wsFrame._frameType);
					break;
				}
				}
			}
		} while(wsFrame._nextReadOffset > 0);

		break;
	}

	default:; // Do nothing
	}

	delete[] data;
	return TcpClient::onReceive(buf);
}

wsMode WebsocketClient::getWSMode()
{
	return _mode;
}
