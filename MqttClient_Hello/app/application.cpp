#include <user_config.h>
#include <SmingCore/SmingCore.h>

// Put you SSID and Password here
#define WIFI_SSID "DontConnect"
#define WIFI_PWD "ignoreme"


Timer procTimer;
MqttClient mqtt("test.mosquitto.org", 1883);

void sendData()
{
	Serial.println("Let's publish message now!");
	mqtt.publishWithQoS("main/frameworks/sming", "Hello friends, from Internet of things :)", 2);
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	mqtt.connect("esp8266");

	// Start send data loop
	procTimer.initializeMs(20 * 1000, sendData).start(); // every 5 seconds
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Disable debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}
