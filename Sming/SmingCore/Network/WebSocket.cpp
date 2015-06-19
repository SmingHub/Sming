/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "WebSocket.h"
#include "../../Services/WebHelpers/aw-sha1.h"
#include "../../Services/WebHelpers/base64.h"
#include "../../Libraries/cWebsocket/websocket.h"

WebSocket::WebSocket(HttpServerConnection* conn)
{
	connection = conn;
}

bool WebSocket::initialize(HttpRequest& request, HttpResponse& response)
{
	String version = request.getHeader("Sec-WebSocket-Version");
	version.trim();
	if (version.toInt() != 13) // 1.3
		return false;

	String hash = request.getHeader("Sec-WebSocket-Key");
	hash.trim();
	hash = hash + secret;
	unsigned char data[SHA1_SIZE];
	char secure[SHA1_SIZE * 4];
	sha1(data, hash.c_str(), hash.length());
	base64_encode(SHA1_SIZE, data, SHA1_SIZE * 4, secure);
	response.switchingProtocols();
	response.setHeader("Connection", "Upgrade");
	response.setHeader("Upgrade", "websocket");
	response.setHeader("Sec-WebSocket-Accept", secure);
	return true;
}

void WebSocket::send(const char* message, int length)
{
	uint8_t frameHeader[16] = {0};
	size_t headSize = sizeof(frameHeader);
	wsMakeFrame(NULL, length, frameHeader, &headSize, WS_TEXT_FRAME);
	connection->write((char*)frameHeader, headSize, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
	connection->writeString(message, TCP_WRITE_FLAG_COPY);
	connection->flush();
}

void WebSocket::sendString(const String& message)
{
	send(message.c_str(), message.length());
}
