#include <SmingCore.h>

#include "configuration.h" // application configuration

#include "bmp180.cpp" // bmp180 configuration
#include "si7021.cpp" // htu21d configuration

Timer publishTimer;

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway);

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	Wire.pins(4, 5); // SDA, SCL
	Wire.begin();

	// initialization config
	mqtt.setCallback(onMessageReceived);

	BMPinit(); // BMP180 sensor initialization
	SIinit();  // HTU21D sensor initialization

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiEvents.onStationGotIP(gotIP);
	WifiAccessPoint.enable(false);
	WDT.enable(false); //disable watchdog
}

// Publish our message
void publishMessage() // uncomment timer in connectOk() if need publishMessage() loop
{
	if(mqtt.getConnectionState() != eTCS_Connected)
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
	Url url(URI_SCHEME_MQTT, F(LOG), F(PASS), F(MQTT_SERVER), MQTT_PORT);
	mqtt.connect(url, CLIENT);
	Serial.println("Connected to MQTT server");
	mqtt.subscribe(SUB_TOPIC);
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	Serial.print("Connected: ");
	Serial.println(ip.toString());
	startMqttClient();
	publishMessage(); // run once publishMessage
}
