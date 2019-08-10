#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

// Will be called when WiFi station network scan was completed
void listNetworks(bool succeeded, BssList& list)
{
	if(!succeeded) {
		Serial.println(_F("Failed to scan networks"));
		return;
	}

	for(unsigned i = 0; i < list.count(); i++) {
		Serial.print(_F("\tWiFi: "));
		Serial.print(list[i].ssid);
		Serial.print(", ");
		Serial.print(list[i].getAuthorizationMethodName());
		if(list[i].hidden) {
			Serial.print(_F(" (hidden)"));
		}
		Serial.println();
	}
}

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial.print(_F("I'm CONNECTED to "));
	Serial.println(ip);
}

// Will be called when WiFi station was disconnected
void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	// The different reason codes can be found in user_interface.h. in your SDK.
	Serial.print(_F("Disconnected from \""));
	Serial.print(ssid);
	Serial.print(_F("\", reason: "));
	Serial.println(WifiEvents.getDisconnectReasonDesc(reason));
}

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready()
{
	Serial.println(_F("READY!"));

	if(WifiAccessPoint.isEnabled()) {
		Serial.print(_F("AP. ip: "));
		Serial.print(WifiAccessPoint.getIP());
		Serial.print(_F(" mac: "));
		Serial.println(WifiAccessPoint.getMacAddress());
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(_F("Sming. Let's do smart things!"));

	// Set system ready callback method
	System.onReady(ready);

	// Soft access point
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config(_F("Sming InternetOfThings"), nullptr, AUTH_OPEN);

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));

	// Optional: Change IP addresses (and disable DHCP)
	WifiAccessPoint.setIP(IpAddress(192, 168, 2, 1));
	WifiStation.setIP(IpAddress(192, 168, 1, 171));

	// Optional: Print details of any incoming probe requests
	WifiEvents.onAccessPointProbeReqRecved([](int rssi, MacAddress mac) {
		Serial.print(_F("Probe request: RSSI = "));
		Serial.print(rssi);
		Serial.print(_F(", mac = "));
		Serial.println(mac);
	});

	// Print available access points
	WifiStation.startScan(listNetworks); // In Sming we can start network scan from init method without additional code

	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);

	// Set callback that should be triggered if we are disconnected or connection attempt failed
	WifiEvents.onStationDisconnect(connectFail);
}
