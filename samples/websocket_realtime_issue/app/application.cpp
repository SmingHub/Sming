#include <SmingCore.h>
#include "webserver.h"

//#define AccessPoint
#ifndef WIFI_SSID
#define WIFI_SSID "YOURSSID" // Put your SSID and password here
#define WIFI_PWD "YOURPASSWORD"
#endif

constexpr uint16_t mainLoopInterval = 20; // ms

SimpleTimer mainLoopTimer;

void mainLoop()
{
	int i = 10;
}
void init()
{
	spiffs_mount();					// Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial
#ifdef AccessPoint
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config(_F("BalanceEsp"), "icanbalance", AUTH_WPA_WPA2_PSK);
#else
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));
	WifiStation.setIP(IpAddress(192, 168, 178, 64));
#endif

	startWebServer();
	mainLoopTimer.initializeMs<mainLoopInterval>(mainLoop).start();
}
