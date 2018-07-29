/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * WebsocketClient
 *
 * @authors:
 * 		 Originally - hrsavla <https://github.com/hrsavla>
 * 		 Refactored - Alexander V, Ribchansky <https://github.com/avr39-ripe>
 * 		 Refactored(2018) - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "WebsocketClient.h"
#include "../../Services/WebHelpers/aw-sha1.h"
#include "../../Services/WebHelpers/base64.h"

bool WebsocketClient::connect(String url, uint32_t sslOptions /* = 0 */)
{
	uri = URL(url);
	bool useSsl = false;
	if(uri.Protocol == WEBSCOKET_SECURE_URL_PROTOCOL) {
		useSsl = true;
	}

	// if we are already connected then do not connect again...
//	if (isProcessing())
//	        return false;

	HttpConnection::connect(uri.Host, uri.Port, useSsl, sslOptions);
	state = WS_STATE_OPENING;

	// Generate the key
	unsigned char keyStart[17];
	char b64Key[25];
	memset(b64Key, 0, sizeof(b64Key));

	for(int i = 0; i < 16; ++i) {
		keyStart[i] = 1 + os_random() % 255;
	}

	base64_encode(16, (const unsigned char*)keyStart, 24, (char*)b64Key);
	key.setString(b64Key, 24);

	HttpRequest* request = new HttpRequest(uri);
	request->headers["Upgrade"] = "websocket";
	request->headers["Connection"] = "Upgrade";
	request->headers["Sec-WebSocket-Key"] = key;
	request->headers["Sec-WebSocket-Protocol"] = "chat";
	request->headers["Sec-WebSocket-Version"] = "13";
	request->onHeadersComplete(RequestHeadersCompletedDelegate(&WebsocketClient::verifyKey, this));

	if(!HttpConnection::send(request)) {
		return false;
	}

	return true;
}

int WebsocketClient::verifyKey(HttpConnection& connection, HttpResponse& response)
{
	if(!response.headers.contains("Sec-WebSocket-Accept")) {
		state = WS_STATE_CLOSING;
		return -2; // we don't have response.
	}

	const char* serverHashedKey = response.headers["Sec-WebSocket-Accept"].c_str();
	unsigned char hashedKey[20];
	char base64HashedKey[20 * 4];
	String keyToHash = key + secret;

	sha1(hashedKey, keyToHash.c_str(), keyToHash.length());
	base64_encode(sizeof(hashedKey), hashedKey, sizeof(base64HashedKey), base64HashedKey);

	if(strcmp(serverHashedKey, base64HashedKey)) {
		debug_e("wscli key mismatch: %s | %s", serverHashedKey, base64HashedKey);
		state = WS_STATE_CLOSING;
		return -3;
	}

	response.headers.clear(); // we don't need the headers any longer
	state = WS_STATE_NORMAL;

	memset(&parserSettings, 0, sizeof(ws_parser_callbacks_t));
	parserSettings.on_data_begin = WebsocketConnection::staticOnDataBegin;
	parserSettings.on_data_payload = WebsocketConnection::staticOnDataPayload;
	parserSettings.on_data_end = WebsocketConnection::staticOnDataEnd;
	parserSettings.on_control_begin = WebsocketConnection::staticOnControlBegin;
	parserSettings.on_control_payload = WebsocketConnection::staticOnControlPayload;
	parserSettings.on_control_end = WebsocketConnection::staticOnControlEnd;

	ws_parser_init(&parser, &parserSettings);
	parser.user_data = (void*)this;

	return 0;
}

void WebsocketClient::onFinished(TcpClientState finishState)
{
	state = WS_STATE_CLOSING;
	if(finishState == eTCS_Failed) {
		debug_e("Tcp Client failure...");
	}

	if(wsDisconnect) {
		wsDisconnect(*this);
	}

	HttpConnection::onFinished(finishState);
}

void WebsocketClient::disconnect()
{
	debug_d("Terminating Websocket connection.");
	state = WS_STATE_CLOSING;

	WebsocketConnection::send(null, 0, WS_FRAME_CLOSE);
}

err_t WebsocketClient::onProtocolUpgrade(http_parser* parser)
{
	debug_d("WebsocketClient::onProtocolUpgrade: Handshake is ready");

	delete HttpConnection::stream;
	WebsocketConnection::userData = (void*)this;
	WebsocketConnection::stream = new EndlessMemoryStream();
	HttpConnection::stream = WebsocketConnection::stream;

	if (wsConnect) {
		wsConnect(*this);
	}

	HttpConnection::onReadyToSendData(eTCE_Received);

	return ERR_OK;
}

err_t WebsocketClient::onReceive(pbuf* buf)
{
	if(buf == NULL || state == WS_STATE_OPENING) {
		return HttpConnection::onReceive(buf);
	}

	pbuf* cur = buf;
	while(cur != nullptr && cur->len > 0) {
		int err = ws_parser_execute(&parser, (char*)cur->payload, cur->len);
		if(err) {
			debug_e("WsClient: Got error: %s", ws_parser_error(err));
			TcpClient::onReceive(nullptr);

			return ERR_ABRT;
		}
		cur = cur->next;
	}

	TcpClient::onReceive(buf);

	return ERR_OK;
}

wsState WebsocketClient::getState()
{
	return state;
}
