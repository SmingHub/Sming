/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_MqttClient_H_
#define _SMING_CORE_NETWORK_MqttClient_H_

#define MQTT_MAX_BUFFER_SIZE 1024

#include "TcpClient.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Services/libemqtt/libemqtt.h"

class MqttClient;
class URL;

class MqttClient: protected TcpClient
{
public:
	MqttClient(String serverHost, int serverPort);
	virtual ~MqttClient();

	bool connect(String clientName);
	bool connect(String clientName, String username, String password);
	bool publish(String topic, String message, bool retained = false);
	bool publishWithQoS(String topic, String message, int QoS, bool retained = false);

protected:
	static int staticSendPacket(void* userInfo, const void* buf, unsigned int count);
	virtual err_t onReceive(pbuf *buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	void debugPrintResponseType(int type);

private:
	String server;
	int port;
	mqtt_broker_handle_t broker;
	int waitingSize;

	uint8_t buffer[MQTT_MAX_BUFFER_SIZE + 1];
	uint8_t *current;
};

#endif /* _SMING_CORE_NETWORK_MqttClient_H_ */
