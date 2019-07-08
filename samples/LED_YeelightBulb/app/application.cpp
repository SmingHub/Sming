#include <SmingCore.h>
#include <YeelightBulb.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

// Enter your bulb IP here:
YeelightBulb bulb(IPAddress("192.168.1.100"));

Timer procTimer;
bool state = false;

void blink()
{
	state = !state;

	if(state) {
		int h = random(0, 360);
		bulb.setHSV(h, 40); // Set color: [HS]V
	}
	bulb.setState(state);
}

// Will be called when WiFi station was connected to AP
void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	debugf("I'm CONNECTED");

	// Connection to Yeelight Bulb will be established on any first action:
	bulb.updateState(); // Read actual bulb state
	procTimer.initializeMs(5000, blink).start();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	WifiEvents.onStationGotIP(gotIP);
}
