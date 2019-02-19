/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebsocketClient.h
 *
 * @authors:
 * 		 Originally - hrsavla <https://github.com/hrsavla>
 * 		 Refactored - Alexander V, Ribchansky <https://github.com/avr39-ripe>
 * 		 Refactored - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

//TODO: Add stream support for sending big chunks of data via websockets.

#ifndef _SMING_CORE_NETWORK_WEBSOCKET_CLIENT_H_
#define _SMING_CORE_NETWORK_WEBSOCKET_CLIENT_H_

#include "Http/HttpConnection.h"
#include "Http/Websocket/WebsocketConnection.h"

/** @defgroup   wsclient Websocket client
 *  @brief      Provides Websocket client
 *  @ingroup    tcpclient
 *  @{
 */

/** @brief  Websocket Client
 */
class WebsocketClient : protected WebsocketConnection
{
public:
	WebsocketClient() : WebsocketConnection(new HttpConnection(new RequestQueue()))
	{
	}

	WebsocketClient(HttpConnectionBase* connection) : WebsocketConnection(connection)
	{
	}

	using WebsocketConnection::setBinaryHandler;
	using WebsocketConnection::setConnectionHandler;
	using WebsocketConnection::setDisconnectionHandler;
	using WebsocketConnection::setMessageHandler;

	HttpConnection* getHttpConnection();

	/**  @brief  Connects websocket client to server
	  *  @param  url URL address of websocket server
	  *  @param  sslOptions Specify the SSL options to be used when calling websocket server over SSL
	  */
	bool connect(const String& url, uint32_t sslOptions = 0);

	using WebsocketConnection::send;
	using WebsocketConnection::sendBinary;
	using WebsocketConnection::sendString;

	/** @brief  Send websocket ping to server
	 *
	 *  @param String payload - maximum 255 bytes
	 *
	 *  @retval bool true if the data can be send, false otherwise
	 */
	void sendPing(const String& payload = nullptr)
	{
		debug_d("Sending PING");
		WebsocketConnection::send(payload.c_str(), payload.length(), WS_FRAME_PING);
	}

	/** @brief  Send websocket ping to server
     *  @param  String& payload  - maximum 255 bytes
     *
     *  @retval bool true if the data can be send, false otherwise
     */
	void sendPong(const String& payload = nullptr)
	{
		debug_d("Sending PONG");
		WebsocketConnection::send(payload.c_str(), payload.length(), WS_FRAME_PONG);
	}

	using WebsocketConnection::close;
	using WebsocketConnection::getState;

	/** @brief  Disconnects websocket client from server
	  * @deprecated Use `close()` instead
	  */
	void disconnect() SMING_DEPRECATED
	{
		close();
	}

protected:
	int verifyKey(HttpConnection& connection, HttpResponse& response);

private:
	URL uri;
	String key;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_WEBSOCKET_CLIENT_H_ */
