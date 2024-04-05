#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
// Will be called when WiFi station network scan was completed
void listNetworks(bool succeeded, BssList& list)
{
	if(!succeeded) {
		Serial.println(_F("Failed to scan networks"));
		return;
	}

	for(auto& bss : list) {
		Serial << _F("\tWiFi: ") << bss.ssid << ", " << bss.getAuthorizationMethodName();
		if(bss.hidden) {
			Serial << _F(" (hidden)");
		}
		Serial.println();
	}
}

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial << _F("I'm CONNECTED to ") << ip << endl;
}

// Will be called when WiFi station was disconnected
void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	// The different reason codes can be found in user_interface.h. in your SDK.
	Serial << _F("Disconnected from \"") << ssid << _F("\", reason: ") << WifiEvents.getDisconnectReasonDesc(reason)
		   << endl;

	/*
	 * Print available access points
	 * 
	 * Note: Calling this in init() *may* work, but it also may result in an error,
	 * "STA is connecting, scan are not allowed!" (ESP32).
	 *
	 * The station interface must be enabled, however, so this is a good place to make the call.
	 * 
	 * A real application would normally do this elsewhere, for example after a user has logged in via AP.
	 */
	WifiStation.startScan(listNetworks); // In Sming we can start network scan from init method without additional code
}

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready()
{
	Serial.println(_F("READY!"));

	if(WifiAccessPoint.isEnabled()) {
		Serial << _F("AP. ip: ") << WifiAccessPoint.getIP() << _F(" mac: ") << WifiAccessPoint.getMacAddress() << endl;
	}
}

} // namespace

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
		Serial << _F("Probe request: RSSI = ") << rssi << _F(", mac = ") << mac << endl;
	});

	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);

	// Set callback that should be triggered if we are disconnected or connection attempt failed
	WifiEvents.onStationDisconnect(connectFail);
}
