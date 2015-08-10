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

String ws_Url =  "ws://echo.websocket.org"; //"ws://192.168.1.2:8080/";
void wsMessageReceived(String message)
{
    Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
}

void wsMessageSent()
{
    if((wsClient.getWSMode() == ws_Connected) )
    {
    	if (msg_cnt >25)
		{
			Serial.println("End Websocket client session");
			wsClient.disconnect(); // clean disconnect.
			msgTimer.stop();
		}
    	else
    	{
			String message = "Hello " + String(msg_cnt++);
			Serial.print("Message to WS Server : ");
			Serial.println(message);
			wsClient.sendMessage(message);
    	}
    }
    else if((wsClient.getWSMode()== ws_Disconnected))
    {
        // Websocket Client is disconnected
        // Restart Connection
        Serial.println("Websocket Client Disconnected. Reconnecting ..");
        wsClient.connect(ws_Url,wsMessageReceived);

    }
    // if websocket client is in ws_Connecting mode then wait till it get connected.
}


void connectOk()
{
    Serial.println("I'm CONNECTED to WiFi");
    Serial.print("Connecting to Websocket Server");
    Serial.println(ws_Url);
    wsClient.connect(ws_Url,wsMessageReceived);
    msgTimer.initializeMs(1 * 1000, wsMessageSent);
    msgTimer.start();
}

void connectFail()
{
    Serial.println("I'm NOT CONNECTED!");
    WifiStation.waitConnection(connectOk, 10, connectFail);
}

void init()
{
    Serial.begin(115200);
    Serial.systemDebugOutput(false);
    WifiAccessPoint.enable(false);

    WifiStation.config(WIFI_SSID, WIFI_PWD);
    WifiStation.enable(true);


    WifiStation.waitConnection(connectOk, 30, connectFail);
}
