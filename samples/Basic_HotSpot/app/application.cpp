#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready()
{
	// Enable your HotSpot
	WifiAccessPoint.enable(true);
	// set the way other devices should see this HotSpot
	WifiAccessPoint.config("SmingAP", "", AUTH_OPEN); // SSID: SmingAP, Open without password
	// If you want more secure HotSpot comment the line above and uncomment the one below
	// WifiAccessPoint.config("SmingAP", "p4ssw0rd!123", AUTH_WPA2_PSK); // SSID: SmingAP, WPA2 authentication with password
}

// Will be called when WiFi station was connected to AP
void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	debugf("I'm CONNECTED to WIFI Router");
	Serial.println(ip.toString());
	ready();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	// Connect first to your WIFI router
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationGotIP(gotIP);
}
