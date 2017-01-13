/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "MqttClient2.h"

MqttClient2::MqttClient2(String serverHost, int serverPort,
		MqttCallback mqttConnectedCallback,
		MqttCallback mqttDisconnectedCallback,
		MqttCallback mqttPublishedCallback, MqttDataCallback mqttDataCallback)

{
	server = serverHost;
	port = serverPort;
	this->mqttConnectedCb = mqttConnectedCallback;
	this->mqttDisconnectedCb = mqttDisconnectedCallback;
	this->mqttPublishedCb = mqttPublishedCallback;
	this->mqttDataCb = mqttDataCallback;

}

MqttClient2::~MqttClient2()
{
}

bool MqttClient2::connect(String clientName)
{
	return MqttClient2::connect(clientName, "", "", 120, true);
}

bool MqttClient2::connect(String idClient, String username, String password,
		int keepAliveTime = 120, bool cleanSession = true)
{
	MQTT_InitConnection(&(this->mqttClient), (uint8_t*) this->server.c_str(),
			1883, DEFAULT_SECURITY);

	MQTT_InitClient(&(this->mqttClient), (uint8_t*) idClient.c_str(),
			(uint8_t*) username.c_str(), (uint8_t*) password.c_str(),
			(uint32_t) keepAliveTime, (uint8) cleanSession);

	MQTT_OnConnected(&(this->mqttClient), this->mqttConnectedCb);
	MQTT_OnDisconnected(&(this->mqttClient), this->mqttDisconnectedCb);
	MQTT_OnPublished(&(this->mqttClient), this->mqttPublishedCb);
	MQTT_OnData(&(this->mqttClient), this->mqttDataCb);

	MQTT_Connect(&(this->mqttClient));

}

void MqttClient2::setLwt(String will_topic, String will_msg, int will_qos,
		bool will_retain)
{
	MQTT_InitLWT(&(this->mqttClient), (uint8_t*) will_topic.c_str(),
			(uint8_t*) will_msg.c_str(), (uint8_t) will_qos,
			(uint8_t) will_retain);
}

bool MqttClient2::publish(String topic, String message, int qos, bool retained)

{
	int res = MQTT_Publish(&(this->mqttClient), topic.c_str(), message.c_str(),
			(int) sizeof(message), (int) qos, (int) retained);

	return res > 0;
}

bool MqttClient2::publishWithQoS(String topic, String message, int qoS,
		bool retained)
{
	int res = MQTT_Publish(&(this->mqttClient), topic.c_str(), message.c_str(),
			(int) sizeof(message), (int) qoS, (int) retained);

	return res > 0;
}

bool MqttClient2::subscribe(String topic, int qos)
{
	uint16_t msgId = 0;
	//debugf("subscription '%s' registered", topic.c_str());
	bool res = MQTT_Subscribe(&(this->mqttClient), (char*) topic.c_str(),
			(uint8_t) 0);
	return res;
}

