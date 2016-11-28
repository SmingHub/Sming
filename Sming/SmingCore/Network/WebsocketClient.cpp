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
	if(_uri.Protocol == WEBSCOKET_SECURE_URL_PROTOCOL) {
		useSsl = true;
	}
	TcpClient::connect(_uri.Host,_uri.Port, useSsl, sslOptions);
	debugf("Connecting to Server");
	unsigned char keyStart[17];
	char b64Key[25];
	_mode = wsMode::Connecting; // Server Connected / WS Upgrade request sent

	randomSeed(analogRead(0));

	for (int i = 0; i < 16; ++i)
	{
		keyStart[i] = (char) random(1, 256);
	}

	base64_encode(16, (const unsigned char*) keyStart, 24, (char*) b64Key);

	for (int i = 0; i < 24; ++i)
	{
		_key[i] = b64Key[i];
	}
	String protocol = "chat";
	sendString("GET ");
	if (_uri.Path != "")
	{
		sendString(_uri.Path);
	}
	else
	{
		sendString("/");
	}
	sendString(" HTTP/1.1\r\n");
	sendString("Upgrade: websocket\r\n");
	sendString("Connection: Upgrade\r\n");
	sendString("Host: ");
	sendString(_uri.Host);
	sendString("\r\n");
	sendString("Sec-WebSocket-Key: ");
	sendString(_key);
	sendString("\r\n");
	sendString("Sec-WebSocket-Protocol: ");
	sendString(protocol);
	sendString("\r\n");
	sendString("Sec-WebSocket-Version: 13\r\n");
	sendString("\r\n", false);
	return true;

}

void WebsocketClient::onError(err_t err)
{
	if ((err == ERR_ABRT) || (err == ERR_RST))
	{
		debugf("TCP Connection Reseted or Aborted", err);
	}
	else
	{
		debugf("Error  %d Occured ", err);
	}
	TcpClient::onError(err);
}

bool WebsocketClient::_verifyKey(char* buf, int size)
{
	String dd = String(buf);
	uint8_t s = dd.indexOf("Sec-WebSocket-Accept: ");
	uint8_t t = dd.indexOf("\r\n", s);
	String serverKey = dd.substring(s + 22, t);
	String hash = _key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	unsigned char data[20];
	char secure[20 * 4];
	sha1(data, hash.c_str(), hash.length());
	base64_encode(20, data, 20 * 4, secure);
	// if the keys match, good to go
	return serverKey.equals(String(secure)); //b64Result
}

void WebsocketClient::onFinished(TcpClientState finishState)
{
	_mode = wsMode::Disconnected;
	uint8_t failed = (finishState == eTCS_Failed);
	if (failed)
	{
		debugf("Tcp Client failure...");
	}
	else
	{
		debugf("Websocket Closed Normally.");

	}

	if (wsDisconnect)
	{
		wsDisconnect(*this, !failed);
	}
	TcpClient::onFinished(finishState);
}

void WebsocketClient::sendPing()
{
	uint8_t buf[2] = { 0x89, 0x00 };
	debugf("Sending PING");
	send((char*) buf, 2, false);
}

void WebsocketClient::sendPong()
{
	uint8_t buf[2] = { 0x8A, 0x00 };
	debugf("Sending PONG");
	send((char*) buf, 2, false);
}

void WebsocketClient::disconnect()
{
	debugf("Terminating Websocket connection.");
	_mode = wsMode::Disconnected;
	// Should send 0x87, 0x00 to server to tell it that I'm quitting here.
	uint8_t buf[2] = { 0x87, 0x00 };
	send((char*) buf, 2, true);
}

void WebsocketClient::_sendFrame(WSFrameType frameType, uint8_t* msg, uint16_t length)
{
	WebsocketFrameClass wsFrame;
	uint8_t result = wsFrame.encodeFrame(frameType, msg, length, true, true, true);

	if (result && wsFrame._header == nullptr)
	{
		send((char*) &wsFrame._payload[0], wsFrame._payloadLength, false);
	}
	else if (result)
	{
		send((char*) &wsFrame._header[0], wsFrame._headerLength, false);
		send((char*) &wsFrame._payload[0], wsFrame._payloadLength, false);
	}
}
void WebsocketClient::sendBinary(uint8_t* msg, uint16_t length)
{
	_sendFrame(WSFrameType::binary, msg, length);
}

void WebsocketClient::sendMessage(char* msg, uint16_t length)
{
	_sendFrame(WSFrameType::text, (uint8_t*) msg, length);
}

void WebsocketClient::sendMessage(String str)
{
	_sendFrame(WSFrameType::text, (uint8_t*) str.c_str(), str.length() + 1);
}

err_t WebsocketClient::onReceive(pbuf* buf)
{
	if (buf == NULL)
	{
		// Disconnected, close it
		TcpClient::onReceive(buf);
	}
	else
	{
		uint16_t size = buf->tot_len;
		uint8_t* data = new uint8_t[size];

		pbuf_copy_partial(buf, data, size, 0);

		switch (_mode)
		{
		case wsMode::Connecting:
			if (_verifyKey((char*)data, size) == true)
			{
				_mode = wsMode::Connected;
				//   debugf("Key Verified. Websocket Handshake completed");
				sendPing();
			}
			else
			{
				_mode = wsMode::Disconnected; // Handshake was not proper.
			}

			if (wsConnect)
			{
				wsConnect(*this, _mode);
			}
			break;

		case wsMode::Connected:
			WebsocketFrameClass wsFrame;
			do
			{
				if (wsFrame.decodeFrame(data, size))
				{
					switch (wsFrame._frameType)
					{
					case WSFrameType::text:
					{
//						debugf("Got text frame");
						String msg;
						msg.setString((char*)wsFrame._payload, wsFrame._payloadLength);
						if (wsMessage)
						{
							wsMessage(*this, msg.c_str());
						}
						break;
					}
					case WSFrameType::binary:
					{
//						debugf("Got binary frame");
						if (wsBinary)
						{
							wsBinary(*this, wsFrame._payload, wsFrame._payloadLength);
						}
						break;
					}
					case WSFrameType::close:
					{
						debugf("Got Disconnect request from server.\n");
						//RFC requires we return a close op code before closing the connection
						disconnect();
						break;
					}
					case WSFrameType::ping:
					{
						debugf("Got ping ...");
						sendPong(); //Need to send Pong in response to Ping
						break;
					}
					case WSFrameType::pong:
					{
						debugf("Got pong ...");
						//A pong can contain app data, but shouldnt if we didnt send any...
						break;
					}
					case WSFrameType::error:
					{
						debugf("ERROR parsing frame!");
						break;
					}
					case WSFrameType::incomplete:
					{
						debugf("INCOMPLETE websocket frame!");
						break;
					}
					default:
					{
						debugf("Unknown frameType: %d", wsFrame._frameType);
						break;
					}
					}
				}
			}
			while (wsFrame._nextReadOffset > 0);

			break;
		}

		delete[] data;
		TcpClient::onReceive(buf);
	}
}

wsMode WebsocketClient::getWSMode()
{
	return _mode;
}
