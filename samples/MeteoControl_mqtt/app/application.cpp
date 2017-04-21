#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "../include/configuration.h"	// application configuration

#include "../app/bmp180.cpp"		// bmp180 configuration
#include "../app/si7021.cpp"		// htu21d configuration

Timer publishTimer;

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway);

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	Wire.pins(5, 4); // SCL, SDA
	Wire.begin();

// initialization config

	BMPinit();	// BMP180 sensor initialization
	SIinit();	// HTU21D sensor initialization

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);
	WDT.enable(false);	//disable watchdog
	System.onReady(systemReady);
}

// Publish our message
void publishMessage()	// uncomment timer in connectOk() if need publishMessage() loop
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.println("publish message");
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
	Serial.println("Connected to MQTT server");
	mqtt.subscribe(SUB_TOPIC);
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	Serial.print("Connected: ");
	Serial.println(ip.toString());
	startMqttClient();
	publishMessage();		// run once publishMessage
}
