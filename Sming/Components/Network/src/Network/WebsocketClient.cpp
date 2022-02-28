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
	if(connection == nullptr && state == State::Closed) {
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

	state = State::Ready;

	// Generate the key
	uint8_t keyStart[16];
	os_get_random(keyStart, sizeof(keyStart));
	key = base64_encode(keyStart, sizeof(keyStart));

	HttpRequest* request = new HttpRequest(uri);
	request->headers[HTTP_HEADER_UPGRADE] = WSSTR_WEBSOCKET;
	request->headers[HTTP_HEADER_CONNECTION] = WSSTR_UPGRADE;
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_KEY] = key;
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL] = F("chat");
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_VERSION] = String(WEBSOCKET_VERSION);
	request->onHeadersComplete(RequestHeadersCompletedDelegate(&WebsocketClient::verifyKey, this));
	request->onRequestComplete([this](HttpConnection& client, bool successful) -> int {
		if(state != State::Open && wsDisconnect) {
			wsDisconnect(*this);
		}
		return 0;
	});

	if(!httpConnection->send(request)) {
		return false;
	}

	WebsocketConnection::setUserData(this);

	return true;
}

int WebsocketClient::verifyKey(HttpConnection& connection, HttpResponse& response)
{
	bool verified{false};

	if(!response.headers.contains(HTTP_HEADER_SEC_WEBSOCKET_ACCEPT)) {
		debug_e("[WS] Websocket Accept missing from headers");
	} else {
		String serverHashedKey = response.headers[HTTP_HEADER_SEC_WEBSOCKET_ACCEPT];
		auto hash = Crypto::Sha1().calculate(key + WSSTR_SECRET);
		String base64hash = base64_encode(hash.data(), hash.size());
		verified = (base64hash == serverHashedKey);
		if(!verified) {
			debug_e("wscli key mismatch: %s | %s", serverHashedKey.c_str(), base64hash.c_str());
		}
	}

	if(!verified) {
		state = State::Closed;
		WebsocketConnection::getConnection()->setTimeOut(1);
		return 0;
	}

	response.headers.clear();

	state = State::Open;
	connection.setTimeOut(USHRT_MAX);
	activate();

	if(wsConnect) {
		wsConnect(*this);
	}

	return 0;
}
