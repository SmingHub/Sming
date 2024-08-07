/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
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

#pragma once

#include "../HttpClientConnection.h"
#include "WebsocketConnection.h"

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
	WebsocketClient(HttpConnection* connection = nullptr) : WebsocketConnection(connection)
	{
	}

	using WebsocketConnection::setBinaryHandler;
	using WebsocketConnection::setConnectionHandler;
	using WebsocketConnection::setDisconnectionHandler;
	using WebsocketConnection::setMessageHandler;

	HttpConnection* getHttpConnection();

	/**  @brief  Connects websocket client to server
	  *  @param  url Url address of websocket server
	  */
	bool connect(const Url& url);

	using WebsocketConnection::send;
	using WebsocketConnection::sendBinary;
	using WebsocketConnection::sendString;

	/** @brief  Send websocket ping to server
	 *
	 *  @param payload Maximum 255 bytes
	 *
	 *  @retval bool true if the data can be send, false otherwise
	 */
	void sendPing(const String& payload = nullptr)
	{
		debug_d("Sending PING");
		WebsocketConnection::send(payload.c_str(), payload.length(), WS_FRAME_PING);
	}

	/** @brief  Send websocket ping to server
     *  @param  payload Maximum 255 bytes
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

	/**
	 * @brief Set the SSL session initialisation callback
	 * @param handler
	 */
	void setSslInitHandler(Ssl::Session::InitDelegate handler)
	{
		sslInitHandler = handler;
	}

protected:
	int verifyKey(HttpConnection& connection, HttpResponse& response);

private:
	Url uri;
	String key;
	Ssl::Session::InitDelegate sslInitHandler;
};

/** @} */
