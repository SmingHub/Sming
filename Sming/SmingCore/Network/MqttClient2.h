/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_MqttClient2_H_
#define _SMING_CORE_NETWORK_MqttClient2_H_

#include "../../Wiring/WString.h"
#include "../../Services/espmqtt/mqtt.h"

class MqttClient2
{
public:
	MqttClient2(String serverHost, int serverPort,
			MqttCallback mqttConnectedCallback = NULL,
			MqttCallback mqttDisconnectedCallback = NULL,
			MqttCallback mqttPublishedCallback = NULL,
			MqttDataCallback mqttDataCallback = NULL);

	virtual ~MqttClient2();

	bool connect(String clientName);
	bool connect(String idClient, String username, String password,
			int keepAliveTime, bool cleanSession);

	void setLwt(String will_topic, String will_msg, int will_qos,
			bool will_retain);

	bool publish(String topic, String message, int qos, bool retained);
	bool publishWithQoS(String topic, String message, int qoS, bool retained);

	bool subscribe(String topic, int qos);

private:
	String server;
	int port;

	MQTT_Client mqttClient;

	MqttCallback mqttConnectedCb;
	MqttCallback mqttDisconnectedCb;
	MqttCallback mqttPublishedCb;
	MqttDataCallback mqttDataCb;

};

#endif /* _SMING_CORE_NETWORK_MqttClient2_H_ */
