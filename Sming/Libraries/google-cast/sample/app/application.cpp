#include <SmingCore.h>
#include <Network/HttpClient.h>
#include <Network/GoogleCast/Client.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

GoogleCast::Client castClient;

bool onMessage(extensions_api_cast_channel_CastMessage message)
{
	// TODO: do something with the response message

	return true;
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

	Serial.println(F("Connecting to your Smart TV"));
	castClient.connect(IpAddress("192.168.10.15"));

	Serial.println(F("Starting YouTube"));
	castClient.launch("YouTube");
	castClient.setResponseHandler(onMessage);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);

	WifiEvents.onStationGotIP(connectOk);
}
