/* Websocket Client Demo
 * By hrsavla https://github.com/hrsavla
 * 08/08/2015
 */
#include <user_config.h>
#include <SmingCore/SmingCore.h>

#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


WebsocketClient wsClient;
Timer msgTimer;

int msg_cnt =0;

String ws_Url = "ws://echo.websocket.org"; //"ws://192.168.0.106:8080/";

void wsMessageReceived(String message)
{
    Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
}

void wsMessageSent()
{
    if((wsClient.getWSMode() == ws_Connected) )
    {
        String message = "Hello " + String(msg_cnt++);
        Serial.print("Message to WS Server : ");
        Serial.println(message);
        wsClient.sendMessage(message);
    }
    else{
        // Websocket Client is disconnected
        // Restart Connection
        Serial.println("Websocket Client Disconnected. Reconnecting ..");
        wsClient.startWS(ws_Url,wsMessageReceived);
    }
}

void connectOk()
{
    debugf("I'm CONNECTED to WiFi");
    wsClient.startWS(ws_Url,wsMessageReceived);
    // send message every 15 seconds
    msgTimer.initializeMs(15 * 1000, wsMessageSent).start();
}

void connectFail()
{
    debugf("I'm NOT CONNECTED!");
    WifiStation.waitConnection(connectOk, 10, connectFail);
}

void init()
{
    Serial.begin(115200);
    Serial.systemDebugOutput(true);
    WifiAccessPoint.enable(false);

    WifiStation.config(WIFI_SSID, WIFI_PWD);
    WifiStation.enable(true);


    WifiStation.waitConnection(connectOk, 30, connectFail);
}
