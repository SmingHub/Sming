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

#ifdef ENABLE_SSL
#include <ssl/private_key.h>
#include <ssl/cert.h>
#define MQTT_URL MQTT_URL2
#else
#define MQTT_URL MQTT_URL1
#endif

// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);

MqttClient mqtt;

Timer procTimer;

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag)
{
	if(flag == true) {
		Serial.println(_F("MQTT Broker Disconnected!!"));
	} else {
		Serial.println(_F("MQTT Broker Unreachable!!"));
	}

	// Restart connection attempt after few seconds
	procTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
}

void onMessageDelivered(uint16_t msgId, int type)
{
	Serial.printf(_F("Message with id %d and QoS %d was delivered successfully."), msgId,
				  (type == MQTT_MSG_PUBREC ? 2 : 1));
}

// Publish our message
void publishMessage()
{
	if(mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient(); // Auto reconnect
	}

	Serial.print(_F("Let's publish message now. Memory free="));
	Serial.println(system_get_free_heap_size());
	mqtt.publish(F("main/frameworks/sming"), F("Hello friends, from Internet of things :)"));

	mqtt.publishWithQoS(F("important/frameworks/sming"), F("Request Return Delivery"), 1, false,
						onMessageDelivered); // or publishWithQoS
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Prettify alignment for printing
	Serial.println(message);
}

// Run MQTT client
void startMqttClient()
{
	procTimer.stop();

	// 1. [Setup]
	if(!mqtt.setWill(F("last/will"), F("The connection from this device is lost:("), 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}

	mqtt.setConnectedHandler([](MqttClient& client, mqtt_message_t* message) {
		Serial.print(_F("Connected to "));
		Serial.println(client.getRemoteIp());
		return 0;
	});

	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.setCallback(onMessageReceived);

#ifdef ENABLE_SSL
	mqtt.setSslInitHandler([](Ssl::Session& session) {
		session.options.verifyLater = true;
		session.keyCert.assign(default_private_key, sizeof(default_private_key), default_certificate,
							   sizeof(default_certificate), nullptr);
	});
#endif

	// 2. [Connect]
	Url url(MQTT_URL);
	Serial.print(_F("Connecting to "));
	Serial.println(url);
	mqtt.connect(url, F("esp8266"));
	mqtt.subscribe(F("main/status/#"));
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
