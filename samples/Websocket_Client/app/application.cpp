/* Websocket Client Demo
 * By hrsavla https://github.com/hrsavla
 * 08/08/2015
 * This is a simple demo of Websocket client
 * Client tries to connect to echo.websocket.org
 * It sents 25 messages then client connection is closed.
 * It reconnects and sends 25 messages and continues doing same.
 *
 * This demo shows connection, closing , reconnection methods of
 * websocket client.
 */
#include <SmingCore.h>
#include <Network/WebsocketClient.h>

#ifndef WIFI_SSID
#define WIFI_SSID "PutSsidHere" // Put your SSID and password here
#define WIFI_PWD "PutPasswordHere"
#endif

//Uncomment next line to enable websocket binary transfer test
//#define WS_BINARY

WebsocketClient wsClient;
Timer msgTimer;
Timer restartTimer;

// Number of messages to send
const unsigned MESSAGES_TO_SEND = 10;

// Interval (in seconds) between sending of messages
const unsigned MESSAGE_INTERVAL = 1;

// Time (in seconds) to wait before restarting client and sending another group of messages
const unsigned RESTART_PERIOD = 20;

unsigned msg_cnt = 0;

#ifdef ENABLE_SSL
DEFINE_FSTR_LOCAL(ws_Url, "wss://echo.websocket.org");
#else
DEFINE_FSTR_LOCAL(ws_Url, "ws://echo.websocket.org");
#endif /* ENABLE_SSL */

void wsMessageSent();

void wsConnected(WebsocketConnection& wsConnection)
{
	Serial << _F("Start sending messages every ") << MESSAGE_INTERVAL << _F(" second(s)...") << endl;
	msgTimer.initializeMs(MESSAGE_INTERVAL * 1000, wsMessageSent);
	msgTimer.start();
}

void wsMessageReceived(WebsocketConnection& wsConnection, const String& message)
{
	Serial << _F("WebSocket message received: ") << message << endl;
	Serial << _F("Free Heap: ") << system_get_free_heap_size() << endl;
}

void wsBinReceived(WebsocketConnection& wsConnection, uint8_t* data, size_t size)
{
	Serial.println(_F("WebSocket BINARY received"));
	for(uint8_t i = 0; i < size; i++) {
		Serial << "wsBin[" << i << "] = 0x" << String(data[i], HEX, 2) << endl;
	}

	Serial << _F("Free Heap: ") << system_get_free_heap_size() << endl;
}

void restart()
{
	Serial.println(_F("restart..."));

	msg_cnt = 0;
	wsClient.connect(String(ws_Url));
}

void wsDisconnected(WebsocketConnection& wsConnection)
{
	Serial << _F("Restarting websocket client after ") << RESTART_PERIOD << _F(" seconds") << endl;
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

	if(msg_cnt > MESSAGES_TO_SEND) {
		Serial.println(_F("End Websocket client session"));
		msgTimer.stop();
		wsClient.close(); // clean disconnect.

		return;
	}

#ifndef WS_BINARY
	String message = F("Hello ") + String(msg_cnt++);
	Serial << _F("Sending websocket message: ") << message << endl;
	wsClient.sendString(message);
#else
	uint8_t buf[] = {0xF0, 0x00, 0xF0};
	buf[1] = msg_cnt++;
	Serial.println(_F("Sending websocket binary buffer"));
	for(uint8_t i = 0; i < 3; i++) {
		Serial << "wsBin[" << i << "] = 0x" << String(buf[i], HEX, 2) << endl;
	}

	wsClient.sendBinary(buf, 3);
#endif
}

void STAGotIP(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial << _F("GOTIP - IP: ") << ip << _F(", MASK: ") << mask << _F(", GW: ") << gateway << endl;

	Serial << _F("Connecting to Websocket Server ") << ws_Url << endl;

	wsClient.setMessageHandler(wsMessageReceived);
	wsClient.setBinaryHandler(wsBinReceived);
	wsClient.setDisconnectionHandler(wsDisconnected);
	wsClient.setConnectionHandler(wsConnected);
	wsClient.setSslInitHandler([](Ssl::Session& session) { session.options.verifyLater = true; });
	wsClient.connect(String(ws_Url));
}

void STADisconnect(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial << _F("DISCONNECT - SSID: ") << ssid << _F(", REASON: ") << WifiEvents.getDisconnectReasonDesc(reason)
		   << endl;
}

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);
	WifiAccessPoint.enable(false);

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	WifiEvents.onStationGotIP(STAGotIP);
	WifiEvents.onStationDisconnect(STADisconnect);
}
