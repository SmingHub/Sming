#include <SmingCore.h>
#include "MqttMessage.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
MqttClient mqtt;

IMPORT_FSTR(privateKeyData, PROJECT_DIR "/files/private.pem.key.der");
IMPORT_FSTR(certificateData, PROJECT_DIR "/files/certificate.pem.crt.der");
IMPORT_FSTR(awsEndpoint, PROJECT_DIR "/files/endpoint.txt");

// Run MQTT client
void startMqttClient()
{
	mqtt.setSslInitHandler([](Ssl::Session& session) {
		session.options.verifyLater = true;
		session.keyCert.assign(privateKeyData, certificateData);
	});

	Url url;
	url.Scheme = URI_SCHEME_MQTT_SECURE;
	url.Host = awsEndpoint;
	mqtt.connect(url, "Basic_AWS");

	// Assign a disconnect callback function
	//	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.subscribe("thing/fish/test");
}

// Publish our message
void publishMessage()
{
	// Auto reconnect
	if(mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient();
	}

	Serial.println("publish message");
	mqtt.publish("version", "ver.1.2");
}

// Callback for messages, arrived from MQTT server
int onMessagePublish(MqttClient& client, mqtt_message_t* message)
{
	Serial.print("Publish: ");
	Serial.print(MqttBuffer(message->publish.topic_name));
	Serial.print(":\r\n\t"); // Pretify alignment for printing
	Serial.println(MqttBuffer(message->publish.content));
	return 0;
}

int onMessageConnect(MqttClient& client, mqtt_message_t* message)
{
	Serial.print("Connect: ");
	Serial.print(MqttBuffer(message->connect.protocol_name));
	Serial.print(", client: ");
	Serial.println(MqttBuffer(message->connect.client_id));
	return 0;
}

int onMessageDisconnect(MqttClient& client, mqtt_message_t* message)
{
	Serial.println("Disconnect");
	return 0;
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print("Connected: ");
	Serial.println(ip);
	startMqttClient();
	publishMessage(); // run once publishMessage

	mqtt.subscribe("$aws/things/Basic_AWS/shadow/get");
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);

	Serial.println("Hello");

	// initialization config
	mqtt.setEventHandler(MQTT_TYPE_PUBLISH, onMessagePublish);
	mqtt.setEventHandler(MQTT_TYPE_CONNECT, onMessageConnect);
	mqtt.setEventHandler(MQTT_TYPE_DISCONNECT, onMessageDisconnect);

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiEvents.onStationGotIP(gotIP);
	WifiAccessPoint.enable(false);
}
