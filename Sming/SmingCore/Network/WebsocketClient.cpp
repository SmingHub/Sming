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

WebsocketClient::WebsocketClient(bool autoDestruct /*= false*/) :
		TcpClient(autoDestruct)
{

}

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

bool WebsocketClient::connect(String url)
{
	this->uri = URL(url);
	this->_url = url;
	TcpClient::connect(uri.Host,uri.Port);
	debugf("Connecting to Server");
	unsigned char keyStart[17];
	char b64Key[25];
	Mode = ws_Connecting; // Server Connected / WS Upgrade request sent

	randomSeed(analogRead(0));

	for (int i = 0; i < 16; ++i)
	{
		keyStart[i] = (char) random(1, 256);
	}

	base64_encode(16, (const unsigned char*) keyStart, 24, (char*) b64Key);

	for (int i = 0; i < 24; ++i)
	{
		key[i] = b64Key[i];
	}
	String protocol = "chat";
	sendString("GET ");
	if (uri.Path != "")
	{
		sendString(uri.Path);
	}
	else
	{
		sendString("/");
	}
	sendString(" HTTP/1.1\r\n");
	sendString("Upgrade: websocket\r\n");
	sendString("Connection: Upgrade\r\n");
	sendString("Host: ");
	sendString(uri.Host);
	sendString("\r\n");
	sendString("Sec-WebSocket-Key: ");
	sendString(key);
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

bool WebsocketClient::verifyKey(char* buf, int size)
{
	String dd = String(buf);
	uint8_t s = dd.indexOf("Sec-WebSocket-Accept: ");
	uint8_t t = dd.indexOf("\r\n", s);
	String serverKey = dd.substring(s + 22, t);
	//debugf("ServerKey : %s",serverKey.c_str());
	String hash = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	unsigned char data[20];
	char secure[20 * 4];
	sha1(data, hash.c_str(), hash.length());
	base64_encode(20, data, 20 * 4, secure);

	//debugf("clienKey : %s",secure);
	// if the keys match, good to go
	return serverKey.equals(String(secure)); //b64Result
}

void WebsocketClient::onFinished(TcpClientState finishState)
{
	Mode = ws_Disconnected;
	if (finishState == eTCS_Failed)
	{
		//  restart();
		debugf("Tcp Client failure...");
		if (wsDisconnect)
		{
			wsDisconnect(*this, false);
		}
	}
	else
	{
		debugf("Websocket Closed Normally.");
		if (wsDisconnect)
		{
			wsDisconnect(*this, true);
		}
	}
	TcpClient::onFinished(finishState);
}

void WebsocketClient::sendPing()
{
	uint8_t buf[2] =
	{ 0x89, 0x00 };
	debugf("Sending PING");
	send((char*) buf, 2, false);
}

void WebsocketClient::sendPong()
{
	uint8_t buf[2] =
	{ 0x8A, 0x00 };
	debugf("Sending PONG");
	send((char*) buf, 2, false);
}

void WebsocketClient::disconnect()
{
	debugf("Terminating Websocket connection.");
	Mode = ws_Disconnected;
	// Should send 0x87, 0x00 to server to tell it that I'm quitting here.
	uint8_t buf[2] =
	{ 0x87, 0x00 };
	send((char*) buf, 2, true);
}

void WebsocketClient::sendFrame(WSFrameType frameType, uint8_t* msg, uint16_t length)
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
	sendFrame(WSFrameType::binary, msg, length);
}

void WebsocketClient::sendMessage(char* msg, uint16_t length)
{
	sendFrame(WSFrameType::text, (uint8_t*) msg, length);
}

void WebsocketClient::sendMessage(String str)
{
	sendFrame(WSFrameType::text, (uint8_t*) str.c_str(), str.length() + 1);
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

		switch (Mode)
		{
		case ws_Connecting:
			if (verifyKey((char*)data, size) == true)
			{
				Mode = ws_Connected;
				//   debugf("Key Verified. Websocket Handshake completed");
				sendPing();
			}
			else
			{
				Mode = ws_Disconnected; // Handshake was not proper.
			}

			if (wsConnect)
			{
				wsConnect(*this, Mode);
			}
			break;

		case ws_Connected:
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
	return Mode;
}
