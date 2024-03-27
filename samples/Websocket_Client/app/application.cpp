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

namespace
{
WebsocketClient wsClient;
Timer msgTimer;

// Number of messages to send
const unsigned MESSAGES_TO_SEND = 5;

// Interval (in seconds) between sending of messages
const unsigned MESSAGE_INTERVAL = 1;

// Time (in seconds) to wait before restarting client and sending another group of messages
const unsigned RESTART_PERIOD = 20;

unsigned messageCount;
bool sendBinary;

DEFINE_FSTR_LOCAL(ws_Url, WS_URL);

void sendNewMessage();

void wsConnected(WebsocketConnection& wsConnection)
{
	Serial << _F("Start sending messages every ") << MESSAGE_INTERVAL << _F(" second(s)...") << endl;
	msgTimer.initializeMs<MESSAGE_INTERVAL * 1000>(sendNewMessage);
	msgTimer.startOnce();
}

void wsMessageReceived(WebsocketConnection& wsConnection, const String& message)
{
	Serial << _F("WebSocket message received: ") << message << endl;
	Serial << _F("Free Heap: ") << system_get_free_heap_size() << endl;
}

void wsBinReceived(WebsocketConnection& wsConnection, uint8_t* data, size_t size)
{
	m_printHex(_F("WebSocket BINARY received"), data, size);
	Serial << _F("Free Heap: ") << system_get_free_heap_size() << endl;
}

void restart()
{
	Serial.println(_F("restart..."));

	messageCount = 0;
	sendBinary = false;
	wsClient.connect(String(ws_Url));
}

void wsDisconnected(WebsocketConnection& wsConnection)
{
	Serial << _F("Restarting websocket client after ") << RESTART_PERIOD << _F(" seconds") << endl;
	msgTimer.initializeMs<RESTART_PERIOD * 1000>(restart);
	msgTimer.startOnce();
}

void sendNewMessage()
{
	if(!WifiStation.isConnected()) {
		// Check if Esp8266 is connected to router
		return;
	}

	if(messageCount >= MESSAGES_TO_SEND) {
		if(sendBinary) {
			Serial.println(_F("End Websocket client session"));
			wsClient.close(); // clean disconnect.
			return;
		}

		messageCount = 0;
		sendBinary = true;
	}

	if(sendBinary) {
		uint8_t buf[10];
		os_get_random(buf, sizeof(buf));
		buf[1] = messageCount;
		m_printHex(_F("Sending websocket binary"), buf, sizeof(buf));
		wsClient.sendBinary(buf, sizeof(buf));
	} else {
		String message = F("Hello ") + String(messageCount);
		Serial << _F("Sending websocket message: ") << message << endl;
		wsClient.sendString(message);
	}

	++messageCount;
	msgTimer.startOnce();
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

} // namespace

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
