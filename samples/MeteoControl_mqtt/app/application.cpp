#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "../include/configuration.h"	// application configuration

#include "../app/bmp180.cpp"		// bmp180 configuration
#include "../app/si7021.cpp"		// htu21d configuration

Timer publishTimer;

void connectOk(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t channel);
void connectFail(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason);
void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway);

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial

	Wire.pins(5, 4); // SCL, SDA
	Wire.begin();

// initialization config

	BMPinit();	// BMP180 sensor initialization
	SIinit();	// HTU21D sensor initialization

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	//WifiStation.setIP(ESP_IP);
	WifiAccessPoint.enable(false);
	WDT.enable(false);	//disable watchdog
	WifiEvents.onStationConnect(connectOk);
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}

// Publish our message
void publishMessage()	// uncomment timer in connectOk() if need publishMessage() loop
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.print("\npublish message");
	mqtt.publish(VER_TOPIC, "ver.1.2"); // or publishWithQoS
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Pretify alignment for printing
	Serial.println(message);
}
// Run MQTT client
void startMqttClient()
{
	mqtt.connect(CLIENT, LOG, PASS);
	Serial.print("\rConnected to MQTT server\n");
	mqtt.subscribe(SUB_TOPIC);
}

// Will be called when WiFi station was connected to AP
void connectOk(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t channel)
{
	debugf("connected");	
}

// Will be called when WiFi station timeout was reached
void connectFail(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
	debugf("connection FAILED");
	Serial.println("NOT CONNECTED");
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	Serial.print("Connected\n");
	Serial.println(ip.toString());
	startMqttClient();
	publishMessage();		// run once publishMessage
}
