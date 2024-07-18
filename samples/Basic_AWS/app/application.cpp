#include <SmingCore.h>
#include <Network/Mqtt/MqttBuffer.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
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
	mqtt.connect(url, F("Basic_AWS"));

	// Assign a disconnect callback function
	//	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.subscribe(F("thing/fish/test"));
}

// Publish our message
void publishMessage()
{
	// Auto reconnect
	if(mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient();
	}

	Serial.println(_F("publish message"));
	mqtt.publish(F("version"), F("ver.1.2"));
}

// Callback for messages, arrived from MQTT server
int onMessagePublish(MqttClient& client, mqtt_message_t* message)
{
	Serial << _F("Publish: ") << MqttBuffer(message->publish.topic_name) << ':' << endl;
	Serial << '\t' << MqttBuffer(message->publish.content) << endl;
	return 0;
}

int onMessageConnect(MqttClient& client, mqtt_message_t* message)
{
	Serial << _F("Connect: ") << MqttBuffer(message->connect.protocol_name) << _F(", client: ")
		   << MqttBuffer(message->connect.client_id) << endl;
	return 0;
}

int onMessageDisconnect(MqttClient& client, mqtt_message_t* message)
{
	Serial.println(_F("Disconnect"));
	return 0;
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial << _F("Connected: ") << ip << endl;
	startMqttClient();
	publishMessage(); // run once publishMessage

	mqtt.subscribe(F("$aws/things/Basic_AWS/shadow/get"));
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);

	Serial.println(_F("Hello"));

	// initialization config
	mqtt.setEventHandler(MQTT_TYPE_PUBLISH, onMessagePublish);
	mqtt.setEventHandler(MQTT_TYPE_CONNECT, onMessageConnect);
	mqtt.setEventHandler(MQTT_TYPE_DISCONNECT, onMessageDisconnect);

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiEvents.onStationGotIP(gotIP);
	WifiAccessPoint.enable(false);
}
