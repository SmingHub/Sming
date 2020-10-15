#include <SmingCore.h>
#include <Network/Ssdp.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Ssdp* ssdp = nullptr;

void startSsdp()
{
	Serial.println(_F("Starting Simple Service Discovery"));
	ssdp = new Ssdp();

	Ssdp::Settings settings;
	settings["friendlyName"] = "Sming Tea Pot";
	settings["deviceType"] = "upnp:tea-pot"; // This device is a tea pot
	ssdp->setSettings(settings);

	HttpResourceTree paths;
	//  TODO: add icon image paths
	ssdp->setWebPaths(paths);

	ssdp->connect();
	ssdp->setResponseHandler([](const HttpHeaders& headers) {
		Serial.println("=========================");
		Serial.printf("Host: %s\n", headers["Host"].c_str());
		Serial.printf("Location: %s\n", headers["Location"].c_str());
		Serial.printf("Search Target: %s\n", headers["ST"].c_str());
	});
	ssdp->search("urn:dial-multiscreen-org:service:dial:1"); // looking for screens
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(_F("Connected. Got IP: "));
	Serial.println(ip);

	startSsdp();
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial.println(F("I'm NOT CONNECTED!"));
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));

	WifiEvents.onStationGotIP(gotIP);
	WifiEvents.onStationDisconnect(connectFail);
}
