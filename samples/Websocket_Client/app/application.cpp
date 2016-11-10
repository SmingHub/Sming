/* Websocket Client Demo
 * By hrsavla https://github.com/hrsavla
 * 08/08/2015
 * This is a simple demo of Websocket client
 * Client tries to connect to echo.websocket.org
 * It sents 25 messages then client connection is closed.
 * It reconnects and sends 25 messages and continues doing same.
 *
 * This demo shows connecton, closing , reconnection methods of
 * websocket client.
 */
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Network/WebsocketClient.h>
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


WebsocketClient wsClient;
Timer msgTimer;
Timer restartTimer;

int msg_cnt =0;

String ws_Url =  "ws://echo.websocket.org"; //"ws://192.168.1.2:8080/";
void wsDisconnected(bool success);
void wsMessageSent();
void wsConnected(wsMode Mode)
{
	if (Mode == ws_Connected)
	{
		msgTimer.initializeMs(1 * 1000, wsMessageSent);
		msgTimer.start();
	}
	else
	{
		Serial.println("Connection with server not successful. Reconnecting..");
		wsClient.connect(ws_Url);
	}
}
void wsMessageReceived(String message)
{
    Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
}

void restart()
{
	msg_cnt = 0;
	wsClient.connect(ws_Url);

	 msgTimer.setCallback(wsMessageSent);
	 msgTimer.setIntervalMs(1*1000);
	 msgTimer.start();
}
void wsDisconnected(bool success)
{
	if (success == true)
	{
		Serial.println("Websocket Client Disconnected Normally. End of program ..");
	}
	else
	{
		Serial.println("Websocket Client Disconnected. Reconnecting ..");

	}
	 msgTimer.setCallback(restart);
	 msgTimer.setIntervalMs(5*1000);
	 msgTimer.startOnce();
}


void wsMessageSent()
{
   if(WifiStation.isConnected() == true)
   { // Check if Esp8266 is connected to router
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

}


void connectOk()
{
    Serial.println("I'm CONNECTED to WiFi");
    Serial.print("Connecting to Websocket Server");
    Serial.println(ws_Url);
    wsClient.setOnReceiveCallback(wsMessageReceived);
    wsClient.setOnDisconnectedCallback(wsDisconnected);
    wsClient.setOnConnectedCallback(wsConnected);
    wsClient.connect(ws_Url);
    
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
