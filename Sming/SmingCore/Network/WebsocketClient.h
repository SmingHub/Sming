/* 
 * File:   WebsocketClient.h
 * Author: https://github.com/hrsavla
 *
 * Created on August 4, 2015, 1:37 PM
 * This Websocket Client library is ported by me into Sming from  
 * https://github.com/MORA99/Stokerbot/tree/master/Libraries/WebSocketClient 
 * 
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

enum wsMode
{
	ws_Disconnected = 0, ws_Connecting, ws_Connected
};

typedef Delegate<void(String message)> WebSocketRxCallback;
typedef Delegate<void(bool  successful)> WebSocketCompleteCallback;
typedef Delegate<void(wsMode  Mode)> WebSocketConnectedCallback;
class WebsocketClient;

class WebsocketClient: protected TcpClient
{
public:
	//  TcpClient wsClient(wsOnCompleted, wsOnReceive);
	WebsocketClient(bool autoDestruct = false); //
	virtual ~WebsocketClient();
	void setOnReceiveCallback(WebSocketRxCallback _rxcallback);
	void setOnDisconnectedCallback(WebSocketCompleteCallback _completecallback);
	void setOnConnectedCallback(WebSocketConnectedCallback _connectedcallback);
	bool connect(String url);//, WebSocketRxCallback _rxcallback = NULL, WebSocketCompleteCallback _completecallback = NULL);
	void sendPing();
	void sendPong();
	void disconnect();
	void sendMessage(char* msg, uint16_t length);
	void sendMessage(String str);
	void sendBinary(uint8_t* msg, uint16_t length);
	wsMode getWSMode();
protected:
	virtual void onFinished(TcpClientState finishState);
	virtual err_t onConnected(err_t err);
	virtual void onError(err_t err);
	virtual err_t onReceive(pbuf *buf);
	bool verifyKey(char *buf, int size);
private:

	URL uri;
	String _url;
	wsMode Mode;
	WebSocketRxCallback rxcallback;
	WebSocketCompleteCallback completecallback;
	WebSocketConnectedCallback connectedcallback;
	bool connected;
	String key;
};

#endif	/* WEBSOCKETCLIENT_H */

