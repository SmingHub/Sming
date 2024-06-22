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

class WebsocketClientConnection : public HttpClientConnection
{
protected:
	// Prevent HttpClientConnection from resetting our receive delegate set by activate() call
	err_t onConnected(err_t err) override
	{
debug_i("%s CONNECTED", __PRETTY_FUNCTION__);

		if(err == ERR_OK) {
			state = eHCS_Ready;
		}

		// IMPORTANT: Skip HttpClientConnection implementation
		return HttpConnection::onConnected(err);
	}

	void onClosed() override
	{
debug_i("%s CLOSED", __PRETTY_FUNCTION__);
HttpClientConnection::onClosed();
	}

	void onError(err_t err) override
	{
debug_i("%s ERROR %d", __PRETTY_FUNCTION__, err);
HttpClientConnection::onError(err);
	}
};

HttpConnection* WebsocketClient::getHttpConnection()
{
	auto connection = WebsocketConnection::getConnection();
	if(connection == nullptr && state == eWSCS_Closed) {
		connection = new WebsocketClientConnection();
		setConnection(connection);
	}

	return connection;
}

bool WebsocketClient::connect(const Url& url)
{
debug_i("%s CONNECT", __PRETTY_FUNCTION__);
	uri = url;
	bool useSsl = (uri.Scheme == URI_SCHEME_WEBSOCKET_SECURE);

	HttpConnection* httpConnection = getHttpConnection();
	if(httpConnection == nullptr) {
debug_i("%s NO CONNECTION", __PRETTY_FUNCTION__);
		return false;
	}

	if(httpConnection->isProcessing()) {
debug_i("%s PROCESSING", __PRETTY_FUNCTION__);
		return false;
	}

	httpConnection->setSslInitHandler(sslInitHandler);
	httpConnection->connect(uri.Host, uri.getPort(), useSsl);

	state = eWSCS_Ready;

	// Generate the key
	unsigned char keyData[16];
	os_get_random(keyData, sizeof(keyData));
	key = base64_encode(keyData, sizeof(keyData));

	HttpRequest* request = new HttpRequest(uri);
	request->headers[HTTP_HEADER_UPGRADE] = WSSTR_WEBSOCKET;
	request->headers[HTTP_HEADER_CONNECTION] = WSSTR_UPGRADE;
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_KEY] = key;
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL] = F("chat");
	request->headers[HTTP_HEADER_SEC_WEBSOCKET_VERSION] = String(WEBSOCKET_VERSION);
	request->onHeadersComplete(RequestHeadersCompletedDelegate(&WebsocketClient::verifyKey, this));
	request->onRequestComplete([this](HttpConnection& client, bool successful) -> int {
		debug_w("XXXXX successful %u", successful);
		if(state != eWSCS_Open && wsDisconnect) {
			wsDisconnect(*this);
		}
		return 0;
	});

	if(!httpConnection->send(request)) {
debug_i("%s SEND", __PRETTY_FUNCTION__);
		return false;
	}

	WebsocketConnection::setUserData(this);

debug_i("%s CONNECTING", __PRETTY_FUNCTION__);

	return true;
}

int WebsocketClient::verifyKey(HttpConnection& connection, HttpResponse& response)
{
debug_i("%s", __PRETTY_FUNCTION__);

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
		debug_e("WS: key mismatch: %s | %s", serverHashedKey.c_str(), base64hash.c_str());
		state = eWSCS_Closed;
		connection.setTimeOut(1);
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
