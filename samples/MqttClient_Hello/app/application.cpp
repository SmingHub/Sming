#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

// For testing purposes, try a few different URL formats
#define MQTT_URL1 "mqtt://attachix.com:1883"
#define MQTT_URL2 "mqtts://attachix.com:8883" // (Need ENABLE_SSL)
#define MQTT_URL3 "mqtt://frank:fiddle@192.168.100.107:1883"

#define MQTT_URL MQTT_URL1

const Url url(MQTT_URL);

// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);

MqttClient mqtt;

Timer procTimer;

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag)
{
	if(flag == true) {
		Serial.println("MQTT Broker Disconnected!!");
	} else {
		Serial.println("MQTT Broker Unreachable!!");
	}

	// Restart connection attempt after few seconds
	procTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
}

void onMessageDelivered(uint16_t msgId, int type)
{
	Serial.printf("Message with id %d and QoS %d was delivered successfully.", msgId,
				  (type == MQTT_MSG_PUBREC ? 2 : 1));
}

// Publish our message
void publishMessage()
{
	if(mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient(); // Auto reconnect
	}

	Serial.print("Let's publish message now. Memory free=");
	Serial.println(system_get_free_heap_size());
	mqtt.publish("main/frameworks/sming", "Hello friends, from Internet of things :)");

	mqtt.publishWithQoS("important/frameworks/sming", "Request Return Delivery", 1, false,
						onMessageDelivered); // or publishWithQoS
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
	procTimer.stop();

	// 1. [Setup]
	if(!mqtt.setWill("last/will", "The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}

	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.setCallback(onMessageReceived);

#ifdef ENABLE_SSL
	mqtt.addSslOptions(SSL_SERVER_VERIFY_LATER);

#include <ssl/private_key.h>
#include <ssl/cert.h>

	mqtt.setSslKeyCert(default_private_key, default_private_key_len, default_certificate, default_certificate_len,
					   nullptr,
					   /*freeAfterHandshake*/ false);

#endif

	// 2. [Connect]
	Url url(MQTT_URL);
	Serial.print("Connecting to \t");
	Serial.println(url);
	mqtt.connect(url, "esp8266");
	mqtt.subscribe("main/status/#");
}

void onConnected(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	// Run MQTT client
	startMqttClient();

	// Start publishing loop
	procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationGotIP(onConnected);
}
