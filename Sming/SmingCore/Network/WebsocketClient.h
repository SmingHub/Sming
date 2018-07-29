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
 * 		 Refactored - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

//TODO: Add stream support for sending big chunks of data via websockets.

#ifndef _SMING_CORE_WEBSOCKETCLIENT_H
#define _SMING_CORE_WEBSOCKETCLIENT_H

#include "Http/HttpConnection.h"
#include "Http/Websocket/WebsocketConnection.h"

/** @defgroup   wsclient WebSocket client
 *  @brief      Provides WebSocket client
 *  @ingroup    tcpclient
 *  @{
 */

/** @brief  Websocket Client
 */
class WebsocketClient : protected WebsocketConnection, protected HttpConnection
{
public:
	WebsocketClient(bool autoDestruct = false) : HttpConnection(new RequestQueue()){};
	virtual ~WebsocketClient(){};

	using WebsocketConnection::setBinaryHandler;
	using WebsocketConnection::setConnectionHandler;
	using WebsocketConnection::setDisconnectionHandler;
	using WebsocketConnection::setMessageHandler;

	/**  @brief  Connects websocket client to server
	  *  @param  url URL address of websocket server
	  *  @param  sslOptions Specify the SSL options to be used when calling websocket server over SSL
	  */
	bool connect(String url, uint32_t sslOptions = 0);

	using WebsocketConnection::send;
	using WebsocketConnection::sendBinary;
	using WebsocketConnection::sendString;

	/** @brief  Send websocket ping to server
	 *
	 *  @param String payload - maximum 255 bytes
	 *
	 *  @retval bool true if the data can be send, false otherwise
	 */
	__forceinline void sendPing(const String& payload = "")
	{
		debug_d("Sending PING");
		WebsocketConnection::send(payload.c_str(), payload.length(), WS_FRAME_PING);
	}

	/** @brief  Send websocket ping to server
     *  @param  String& payload  - maximum 255 bytes
     *
     *  @retval bool true if the data can be send, false otherwise
     */
	__forceinline void sendPong(const String& payload = "")
	{
		debug_d("Sending PONG");
		WebsocketConnection::send(payload.c_str(), payload.length(), WS_FRAME_PONG);
	}

	/** @brief  Disconnects websocket client from server
	  */
	void disconnect();

	/** @brief  Get websocket client mode
	  *  @retval Return websocket client mode
	  */
	wsState getState();

#ifdef ENABLE_SSL
	using TcpClient::addSslOptions;
	using TcpClient::addSslValidator;
	using TcpClient::freeSslKeyCert;
	using TcpClient::getSsl;
	using TcpClient::pinCertificate;
	using TcpClient::setSslKeyCert;
#endif

protected:
	int verifyKey(HttpConnection& connection, HttpResponse& response);

	virtual void onFinished(TcpClientState finishState);
	virtual err_t onReceive(pbuf* buf);
	virtual err_t onProtocolUpgrade(http_parser* parser);

private:
	URL uri;
	wsState state = WS_STATE_OPENING;
	String key;

	ws_parser_t parser;
	ws_parser_callbacks_t parserSettings;
};

/** @} */
#endif /* _SMING_CORE_WEBSOCKETCLIENT_H */
