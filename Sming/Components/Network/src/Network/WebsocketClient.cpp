/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebsocketClient.cpp
 *
 * @authors:
 * 		 Originally - hrsavla <https://github.com/hrsavla>
 * 		 Refactored - Alexander V, Ribchansky <https://github.com/avr39-ripe>
 * 		 Refactored(2018) - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "WebsocketClient.h"
#include "Http/HttpHeaders.h"
#include <Data/WebHelpers/base64.h>

HttpConnection* WebsocketClient::getHttpConnection()
{
	auto connection = WebsocketConnection::getConnection();
	if(connection == nullptr && state == eWSCS_Closed) {
		connection = new HttpClientConnection();
		setConnection(connection);
	}

	return connection;
}

bool WebsocketClient::connect(const Url& url)
{
	uri = url;
	bool useSsl = (uri.Scheme == URI_SCHEME_WEBSOCKET_SECURE);

	HttpConnection* httpConnection = getHttpConnection();
	if(httpConnection == nullptr) {
		return false;
	}

	if(httpConnection->isProcessing()) {
		return false;
	}

	httpConnection->setSslInitHandler(sslInitHandler);
	httpConnection->connect(uri.Host, uri.getPort(), useSsl);

	state = eWSCS_Ready;

	// Generate the key
	unsigned char keyStart[17] = {0};
	char b64Key[25];
	memset(b64Key, 0, sizeof(b64Key));

	for(int i = 0; i < 16; ++i) {
		keyStart[i] = 1 + os_random() % 255;
	}
	key = base64_encode(keyStart, sizeof(keyStart));

	HttpRequest* request = new HttpRequest(uri);
	request->headers[HTTP_HEADER_UPGRADE] = WSSTR_WEBSOCKET;
	request->headers[HTTP_HEADER_CONNECTION] = WSSTR_UPGRADE;
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_KEY] = key;
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL] = F("chat");
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_VERSION] = String(WEBSOCKET_VERSION);
	request->onHeadersComplete(RequestHeadersCompletedDelegate(&WebsocketClient::verifyKey, this));

	if(!httpConnection->send(request)) {
		return false;
	}

	WebsocketConnection::setUserData(this);

	return true;
}

int WebsocketClient::verifyKey(HttpConnection& connection, HttpResponse& response)
{
	if(!response.headers.contains(HTTP_HEADER_SEC_WEBSOCKET_ACCEPT)) {
		debug_e("[WS] Websocket Accept missing from headers");
		state = eWSCS_Closed;
		return -2; // we don't have response.
	}

	String serverHashedKey = response.headers[HTTP_HEADER_SEC_WEBSOCKET_ACCEPT];

	String keyToHash = key + WSSTR_SECRET;
	auto hash = Crypto::Sha1().calculate(keyToHash);
	String base64hash = base64_encode(hash.data(), hash.size());
	if(base64hash != serverHashedKey) {
		debug_e("wscli key mismatch: %s | %s", serverHashedKey.c_str(), base64hash.c_str());
		state = eWSCS_Closed;
		WebsocketConnection::getConnection()->setTimeOut(1);
		return -3;
	}

	response.headers.clear();

	state = eWSCS_Open;
	connection.setTimeOut(USHRT_MAX);
	activate();

	if(wsConnect) {
		wsConnect(*this);
	}

	return 0;
}
