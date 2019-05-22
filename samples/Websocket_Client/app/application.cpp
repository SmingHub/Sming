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
#include <SmingCore.h>
#include <Network/WebsocketClient.h>

#ifndef WIFI_SSID
#define WIFI_SSID "PutSsidHere" // Put you SSID and Password here
#define WIFI_PWD "PutPasswordHere"
#endif

//Uncomment next line to enable websocket binary transfer test
//#define WS_BINARY

WebsocketClient wsClient;
Timer msgTimer;
Timer restartTimer;

#define RESTART_PERIOD 20

int msg_cnt = 0;

#ifdef ENABLE_SSL
String ws_Url = "wss://echo.websocket.org";
#else
String ws_Url = "ws://echo.websocket.org";
#endif /* ENABLE_SSL */

void wsDisconnected(WebsocketConnection& wsConnection, bool success);
void wsMessageSent();

void wsConnected(WebsocketConnection& wsConnection)
{
	Serial.printf("Start sending messages every second...");
	msgTimer.initializeMs(1 * 1000, wsMessageSent);
	msgTimer.start();
}

void wsMessageReceived(WebsocketConnection& wsConnection, const String& message)
{
	Serial.printf("WebSocket message received: %s\n", message.c_str());
	Serial.printf("Free Heap: %d\n", system_get_free_heap_size());
}

void wsBinReceived(WebsocketConnection& wsConnection, uint8_t* data, size_t size)
{
	Serial.printf("WebSocket BINARY received\n");
	for(uint8_t i = 0; i < size; i++) {
		Serial.printf("wsBin[%u] = 0x%02X\n", i, data[i]);
	}

	Serial.printf("Free Heap: %d\n", system_get_free_heap_size());
}

void restart()
{
	msg_cnt = 0;
	wsClient.connect(ws_Url);
#ifdef ENABLE_SSL
	wsClient.getHttpConnection()->addSslOptions(SSL_SERVER_VERIFY_LATER);
#endif

	msgTimer.setCallback(wsMessageSent);
	msgTimer.setIntervalMs(1 * 1000);
	msgTimer.start();
}

void wsDisconnected(WebsocketConnection& wsConnection)
{
	Serial.printf("Restarting websocket client after %d seconds\n", RESTART_PERIOD);
	msgTimer.setCallback(restart);
	msgTimer.setIntervalMs(RESTART_PERIOD * 1000);
	msgTimer.startOnce();
}

void wsMessageSent()
{
	if(!WifiStation.isConnected()) {
		// Check if Esp8266 is connected to router
		return;
	}

	if(msg_cnt > 10) {
		Serial.println("End Websocket client session");
		wsClient.close(); // clean disconnect.
		msgTimer.stop();

		return;
	}

#ifndef WS_BINARY
	String message = "Hello " + String(msg_cnt++);
	Serial.printf("Sending websocket message: %s\n", message.c_str());
	wsClient.sendString(message);
#else
	uint8_t buf[] = {0xF0, 0x00, 0xF0};
	buf[1] = msg_cnt++;
	Serial.printf("Sending websocket binary buffer\n");
	for(uint8_t i = 0; i < 3; i++) {
		Serial.printf("wsBin[%u] = 0x%02X\n", i, buf[i]);
	}

	wsClient.sendBinary(buf, 3);
#endif
}

void STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway)
{
	Serial.printf("GOTIP - IP: %s, MASK: %s, GW: %s\n", ip.toString().c_str(), mask.toString().c_str(),
				  gateway.toString().c_str());

	Serial.printf("Connecting to Websocket Server %s\n", ws_Url.c_str());

	wsClient.setMessageHandler(wsMessageReceived);
	wsClient.setBinaryHandler(wsBinReceived);
	wsClient.setDisconnectionHandler(wsDisconnected);
	wsClient.setConnectionHandler(wsConnected);
	wsClient.connect(ws_Url);
#ifdef ENABLE_SSL
	wsClient.getHttpConnection()->addSslOptions(SSL_SERVER_VERIFY_LATER);
#endif
}

void STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
	Serial.printf("DISCONNECT - SSID: %s, REASON: %d\n", ssid.c_str(), reason);
}

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(false);
	WifiAccessPoint.enable(false);

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	WifiEvents.onStationGotIP(STAGotIP);
	WifiEvents.onStationDisconnect(STADisconnect);
}
