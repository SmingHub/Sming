#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "../include/configuration.h"	// application configuration

#include "../app/bmp180.cpp"		// bmp180 configuration
#include "../app/si7021.cpp"		// htu21d configuration

Timer publishTimer;

void connectOk();
void connectFail();

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
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
	
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
void connectOk()
{
	debugf("connected");
	Serial.print("Connected\n");
	Serial.println(WifiStation.getIP().toString());
	startMqttClient();
	publishMessage();		// run once publishMessage
	
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("connection FAILED");
	Serial.println("NOT CONNECTED");

	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

