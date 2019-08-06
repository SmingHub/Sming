#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer procTimer;
int sensorValue = 0;
HttpClient thingSpeak;

int onDataSent(HttpConnection& client, bool successful)
{
	if(successful)
		Serial.println("Success sent");
	else
		Serial.println("Failed");

	String response = client.getResponse()->getBody();
	Serial.println("Server response: '" + response + "'");
	if(response.length() > 0) {
		int intVal = response.toInt();

		if(intVal == 0)
			Serial.println("Sensor value wasn't accepted. May be we need to wait a little?");
	}

	return 0;
}

void sendData()
{
	// Read our sensor value :)
	sensorValue++;

	/*
	   Here is an alternative method of URL construction, which is helpful for more complex URLs.
	   The resulting URL string is equivalent to:

	     "http://api.thingspeak.com/update?key=7XXUJWCWYTMXKN3L&field1=" + String(sensorValue)
	 */
	Url url;
	url.Host = "api.thingspeak.com";
	url.Path = "/update";
	url.Query["key"] = "7XXUJWCWYTMXKN3L";
	url.Query["field1"] = String(sensorValue);
	thingSpeak.downloadString(url, onDataSent);
}

// Will be called when WiFi station timeout was reached
void connectFail(const String& ssid, const MACAddress& bssid, WifiDisconnectReason reason)
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// Start soft access point
	WifiAccessPoint.config("CONFIG ME PLEEEEASE...", "", AUTH_OPEN);
	WifiAccessPoint.enable(true);

	// .. some you code for configuration ..
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	// Start send data loop
	procTimer.initializeMs(25 * 1000, sendData).start(); // every 25 seconds
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
