#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer procTimer;

void mqttConnectedCb(uint32_t *args);
void mqttDisconnectedCb(uint32_t *args);
void mqttPublishedCb(uint32_t *args);
void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len,
		const char *data, uint32_t data_len);

MqttClient2 mqtt("mqtt.yourserver.com", 1883, mqttConnectedCb, mqttDisconnectedCb,
		mqttPublishedCb, mqttDataCb);

void publishMessage()
{
	m_printf("Let's publish message now!\r\n");
	mqtt.publish("main/frameworks/sming",
			"Hello friends, from Internet of things :)", 0, false);
}

void mqttConnectedCb(uint32_t *args)
{

	m_printf("MQTT: Connected\r\n");
	mqtt.subscribe("#", 1);

}

void mqttDisconnectedCb(uint32_t *args)
{
	m_printf("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{

	m_printf("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len,
		const char *data, uint32_t data_len)
{
	char *topicBuf = (char*) os_zalloc(topic_len + 1), *dataBuf =
			(char*) os_zalloc(data_len + 1);

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	m_printf("Receive topic:%s, data: %s \r\n", topicBuf, dataBuf);

	os_free(topicBuf);
	os_free(dataBuf);
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	m_printf("WiFi station is connected to AP\r\n");
	mqtt.connect("mqtt_esp", "user", "password", 120, 0);

	// Start publishing loop
	procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	m_printf("I'm NOT CONNECTED. Need help :(");

// .. some you code for device configuration ..
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

// If the next line is comment, previous parameters will be reused
//WifiStation.config(WIFI_SSID, WIFI_PWD);

	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}
