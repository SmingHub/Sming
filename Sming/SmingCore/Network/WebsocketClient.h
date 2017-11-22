/* 
 * File:   WebsocketClient.h
 * Author: https://github.com/hrsavla
 *
 * Created on August 4, 2015, 1:37 PM
 * This Websocket Client library is ported by hrsavla into Sming from
 * https://github.com/MORA99/Stokerbot/tree/master/Libraries/WebSocketClient 
 * 
 * Refactor and improve by https://github.com/avr39-ripe - Alexander V, Ribchansky
 * 
 */

/** @defgroup   wsclient WebSocket client
 *  @brief      Provides WebSocket client
 *  @ingroup    tcpclient
 *  @{
 */

#ifndef WEBSOCKETCLIENT_H
#define	WEBSOCKETCLIENT_H

#include "../Wiring/WiringFrameworkIncludes.h"
#include "TcpClient.h"
#include "../Delegate.h"
#include "URL.h"
#include "../../Services/WebHelpers/aw-sha1.h"
#include "../../Services/WebHelpers/base64.h"
#include "../../Wiring/WString.h"
#include "../Digital.h"
#include <Network/WebsocketFrame.h>

/**	@defgroup wsclient Websocket Client
 *	@brief	Client to make persistent connection to websocket server
 *  @{
 */
class WebsocketClient;
/** @brief  Websocket client modes enum
 */
enum class wsMode : uint8_t
{
	Disconnected = 0, Connecting, Connected
};

typedef Delegate<void(WebsocketClient& wsClient, const String message)> WebSocketClientMessageDelegate;
/** @brief  Delegate callback type for received text message
 */
typedef Delegate<void(WebsocketClient& wsClient, uint8_t* data, size_t size)> WebSocketClientBinaryDelegate;
/** @brief  Delegate callback type for received binary message
 */
typedef Delegate<void(WebsocketClient& wsClient, bool  successful)> WebSocketClientDisconnectDelegate;
/** @brief  Delegate callback type for disconnection of websocket client
 */
typedef Delegate<void(WebsocketClient& wsClient, wsMode  Mode)> WebSocketClientConnectedDelegate;
/** @brief  Delegate callback type for connection of websocket client
 */

/** @brief  Websocket Client
 */
class WebsocketClient: protected TcpClient
{
public:
	WebsocketClient(bool autoDestruct = false) :TcpClient(autoDestruct) {};
	virtual ~WebsocketClient() {};

	/** @brief  Set handler for websocket text messages
	 *  @param  handler Delegate callback to be run when text message received
	 */
	void setWebSocketMessageHandler(WebSocketClientMessageDelegate handler);

	/** @brief  Set handler for websocket disconnection event
	 *  @param  handler Delegate callback to be run when websocket disconnects
	 */
	void setWebSocketDisconnectedHandler(WebSocketClientDisconnectDelegate handler);

	/** @brief  Set handler for websocket connection event
	 *  @param  handler Delegate callback to be run when websocket connects
	 */
	void setWebSocketConnectedHandler(WebSocketClientConnectedDelegate handler);

	 /** @brief  Set handler for websocket binary messages
	  *  @param  handler Delegate callback to be run when binary message received
	  */
	void setWebSocketBinaryHandler(WebSocketClientBinaryDelegate handler);

	/** @brief  Connects websocket client to server
	  *  @param  url URL address of websocket server
	  *  @param  sslOptions Specify the SSL options to be used when calling websocket server over SSL
	  */
	bool connect(String url, uint32_t sslOptions = 0);

	 /** @brief  Send websocket ping to server
	  *
	  *  @param String payload - maximum 255 bytes
	  *
	  *  @retval bool true if the data can be send, false otherwise
	  */
	bool sendPing(const String& payload = "");

	 /** @brief  Send websocket ping to server
	  *  @param  String& payload  - maximum 255 bytes
	  *
	  *  @retval bool true if the data can be send, false otherwise
	  */
	bool sendPong(const String& payload = "");

	/** @brief  Disconnects websocket client from server
	  */
	void disconnect();

	/** @brief  Send text message to websocket server
	  *  @param  msg Pointer to NULL-terminated string buffer to be send to websocket server
	  *  @param  length length of the NULL-terminated string buffer
	  */
	void sendMessage(char* msg, uint16_t length);

	/** @brief  Send text message to websocket server
	  *  @param  C++ String to be send to websocket server
	  */
	void sendMessage(const String& str);

	/**  @brief  Send binary message to websocket server
	  *  @param  msg Pointer to binary-data buffer to be send to websocket server
	  *  @param  length length of the binary-data buffer
	  */
	void sendBinary(uint8_t* msg, uint16_t length);

	/** @brief  Send control frame to websocket server
	  *  @param  payload C++ String to be send to websocket server
	  *
	  */
	bool sendControlFrame(WSFrameType frameType, const String& payload = "");

	/** @brief  Get websocket client mode
	  *  @retval Returnt websocket client mode
	  */
	wsMode getWSMode();

#ifdef ENABLE_SSL
	using TcpClient::addSslOptions;
	using TcpClient::setSslFingerprint;
	using TcpClient::pinCertificate;
	using TcpClient::setSslClientKeyCert;
	using TcpClient::freeSslClientKeyCert;
	using TcpClient::getSsl;
#endif

protected:
	virtual void onFinished(TcpClientState finishState);
	virtual void onError(err_t err);
	virtual err_t onReceive(pbuf *buf);
	bool _verifyKey(char *buf, int size);
	void _sendFrame(WSFrameType frameType, uint8_t* msg, uint16_t length);
private:
	URL _uri;
	String _url;
	wsMode _mode = wsMode::Disconnected;
	WebSocketClientMessageDelegate wsMessage = nullptr;
	WebSocketClientBinaryDelegate wsBinary = nullptr;
	WebSocketClientDisconnectDelegate wsDisconnect = nullptr;
	WebSocketClientConnectedDelegate wsConnect = nullptr;
	String _key;
};

/** @} */
#endif	/* WEBSOCKETCLIENT_H */

