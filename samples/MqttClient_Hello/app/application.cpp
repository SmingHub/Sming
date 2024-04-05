#include <SmingCore.h>
#include <Network/Mqtt/MqttBuffer.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
// For testing purposes, try a few different URL formats
DEFINE_FSTR(MQTT_URL1, "mqtt://test.mosquitto.org:1883")
DEFINE_FSTR(MQTT_URL2, "mqtts://test.mosquitto.org:8883") // (Need ENABLE_SSL)
DEFINE_FSTR(MQTT_URL3, "mqtt://frank:fiddle@192.168.100.107:1883")

#ifdef ENABLE_SSL
#include <ssl/private_key.h>
#include <ssl/cert.h>
#define MQTT_URL MQTT_URL2
#else
#define MQTT_URL MQTT_URL1
#endif

// Forward declarations
void startMqttClient();

MqttClient mqtt;
SimpleTimer procTimer;

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag)
{
	if(flag == true) {
		Serial.println(_F("MQTT Broker Disconnected!!"));
	} else {
		Serial.println(_F("MQTT Broker Unreachable!!"));
	}

	// Restart connection attempt after few seconds
	procTimer.initializeMs<2 * 1000>(startMqttClient).start(); // every 2 seconds
}

int onMessageDelivered(MqttClient& client, mqtt_message_t* message)
{
	Serial << _F("Message with id ") << message->puback.message_id << _F(" and QoS ") << message->puback.qos
		   << _F(" was delivered successfully.") << endl;
	return 0;
}

// Publish our message
void publishMessage()
{
	if(mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient(); // Auto reconnect
	}

	Serial << _F("Let's publish message now. Memory free=") << system_get_free_heap_size() << endl;
	mqtt.publish(F("main/frameworks/sming"), F("Hello friends, from Internet of things :)"));

	mqtt.publish(F("important/frameworks/sming"), F("Request Return Delivery"),
				 MqttClient::getFlags(MQTT_QOS_AT_LEAST_ONCE));
}

// Callback for messages, arrived from MQTT server
int onMessageReceived(MqttClient& client, mqtt_message_t* message)
{
	Serial << _F("Received: ") << MqttBuffer(message->publish.topic_name) << ':' << endl;
	Serial << '\t' << MqttBuffer(message->publish.content) << endl;
	return 0;
}

// Run MQTT client
void startMqttClient()
{
	procTimer.stop();

	// 1. [Setup]
	if(!mqtt.setWill(F("last/will"), F("The connection from this device is lost:("),
					 MqttClient::getFlags(MQTT_QOS_AT_LEAST_ONCE, MQTT_RETAIN_TRUE))) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}

	mqtt.setEventHandler(MQTT_TYPE_PUBACK, onMessageDelivered);

	mqtt.setConnectedHandler([](MqttClient& client, mqtt_message_t* message) {
		Serial << _F("Connected to ") << client.getRemoteIp() << endl;

		// Start publishing message now
		publishMessage();
		// and schedule a timer to send messages every 5 seconds
		procTimer.initializeMs<5 * 1000>(publishMessage).start();
		return 0;
	});

	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.setMessageHandler(onMessageReceived);

#ifdef ENABLE_SSL
	mqtt.setSslInitHandler([](Ssl::Session& session) {
		session.options.verifyLater = true;
		session.keyCert.assign(default_private_key, sizeof(default_private_key), default_certificate,
							   sizeof(default_certificate), nullptr);
	});
#endif

	// 2. [Connect]
	Url url(MQTT_URL);
	Serial << _F("Connecting to ") << url << endl;
	mqtt.connect(url, F("esp8266"));
	mqtt.subscribe(F("main/status/#"));
}

void onConnected(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.println(_F("WIFI connected. Starting MQTT client..."));

	// Run MQTT client
	startMqttClient();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationGotIP(onConnected);
}
