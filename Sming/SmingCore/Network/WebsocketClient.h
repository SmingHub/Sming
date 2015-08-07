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
#include <user_config.h>
#include "../SmingCore.h"
#include "TcpClient.h"
#include "../Delegate.h"
#include "URL.h"
#include "../../Services/WebHelpers/aw-sha1.h"
#include "../../Services/WebHelpers/base64.h"
#include "../../Wiring/WString.h"
#include "../PWM.h"

typedef Delegate<void(String message)> WebSocketRxCallback;
enum wsMode
{
	ws_Disconnected = 0,
	ws_Connecting,
	ws_Connected
};
class WebsocketClient : protected TcpClient
{
public:
   //  TcpClient wsClient(wsOnCompleted, wsOnReceive);
    WebsocketClient(bool autoDestruct = false);//
    virtual ~WebsocketClient();
    bool startWS(String url, WebSocketRxCallback _callback = NULL);
    void sendPing();
    void sendPong();
    void endWS();
    void sendMessage(char* msg, uint16_t length);
    void sendMessage(String str);
    void sendBinary(uint8_t* msg, uint16_t length);
    wsMode getWSMode();
protected:
    void onFinished(TcpClientState finishState);
    virtual err_t onReceive(pbuf *buf);
    void restart();
    bool verifyKey(char *buf, int size);
private:
       
        URL uri;
        String _url;
        wsMode Mode ;
	WebSocketRxCallback callback;
        bool connected;
        String key;
};

#endif	/* WEBSOCKETCLIENT_H */

