#include <user_config.h>
#include <SmingCore/SmingCore.h>

// Will be called when WiFi station network scan was completed
void listNetworks(bool succeeded, BssList list)
{
	if (!succeeded)
	{
		debugf("Failed to scan networks");
		return;
	}

	for (int i = 0; i < list.count(); i++)
	{
		Serial.print("\tWiFi: ");
		Serial.print(list[i].ssid);
		Serial.print(", ");
		Serial.print(list[i].getAuthorizationMethodName());
		if (list[i].hidden) Serial.print(" (hidden)");
		Serial.println();
	}
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	debugf("I'm CONNECTED");
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat check again
}

// Will be called when WiFi hardware and software initialization was finished
void ready()
{
	debugf("READY!");

	// If AP is enabled:
	debugf("%d %s", WifiAccessPoint.getIP(), WifiAccessPoint.getMAC().c_str());
}

void init()
{
	Serial.begin(230400);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Hello friendly world! :)");

	// Set system ready callback method
	System.onReady(ready);

	// Soft access point
	WifiAccessPoint.config("Sming InternetOfThings", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);

	// Station - WiFi client
	WifiStation.config("LOCAL-NETWORK", "123456789087"); // Put you SSID and Password here
	WifiStation.enable(true);

	// Change AP IP
	ip_addr ip;
	IP4_ADDR(&ip, 192, 168, 2, 1);
	WifiAccessPoint.setIP(ip);

	// Print available access points
	WifiStation.startScan(listNetworks); // In Sming we can start network scan from init method without additional code

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
}
