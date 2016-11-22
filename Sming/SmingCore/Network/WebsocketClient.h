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

class WebsocketClient;

enum wsMode
{
	ws_Disconnected = 0, ws_Connecting, ws_Connected
};

typedef Delegate<void(WebsocketClient& wsClient, const String message)> WebSocketClientMessageDelegate;
typedef Delegate<void(WebsocketClient& wsClient, uint8_t* data, size_t size)> WebSocketClientBinaryDelegate;
typedef Delegate<void(WebsocketClient& wsClient, bool  successful)> WebSocketClientDisconnectDelegate;
typedef Delegate<void(WebsocketClient& wsClient, wsMode  Mode)> WebSocketClientConnectedDelegate;
class WebsocketClient;

class WebsocketClient: protected TcpClient
{
public:
	//  TcpClient wsClient(wsOnCompleted, wsOnReceive);
	WebsocketClient(bool autoDestruct = false); //
	virtual ~WebsocketClient() {};
	void setWebSocketMessageHandler(WebSocketClientMessageDelegate handler);
	void setWebSocketDisconnectedHandler(WebSocketClientDisconnectDelegate handler);
	void setWebSocketConnectedHandler(WebSocketClientConnectedDelegate handler);
	void setWebSocketBinaryHandler(WebSocketClientBinaryDelegate handler);
	bool connect(String url);
	void sendPing();
	void sendPong();
	void disconnect();
	void sendMessage(char* msg, uint16_t length);
	void sendMessage(String str);
	void sendBinary(uint8_t* msg, uint16_t length);
	void sendFrame(WSFrameType frameType, uint8_t* msg, uint16_t length);
	wsMode getWSMode();
protected:
	virtual void onFinished(TcpClientState finishState);
	virtual void onError(err_t err);
	virtual err_t onReceive(pbuf *buf);
	bool verifyKey(char *buf, int size);
private:

	URL uri;
	String _url;
	wsMode Mode = ws_Disconnected;
	WebSocketClientMessageDelegate wsMessage = nullptr;
	WebSocketClientBinaryDelegate wsBinary = nullptr;
	WebSocketClientDisconnectDelegate wsDisconnect = nullptr;
	WebSocketClientConnectedDelegate wsConnect = nullptr;
	bool connected = false;
	String key;
};

#endif	/* WEBSOCKETCLIENT_H */

