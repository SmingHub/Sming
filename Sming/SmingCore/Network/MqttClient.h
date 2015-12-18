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
#include "../Delegate.h"
#include "../../Wiring/WString.h"
#include "../../Services/libemqtt/libemqtt.h"

//typedef void (*MqttStringSubscriptionCallback)(String topic, String message);
typedef Delegate<void(String topic, String message)> MqttStringSubscriptionCallback;

class MqttClient;
class URL;

class MqttClient: protected TcpClient
{
public:
	MqttClient(String serverHost, int serverPort, MqttStringSubscriptionCallback callback = NULL);
	MqttClient(IPAddress serverIp, int serverPort, MqttStringSubscriptionCallback callback = NULL);
	virtual ~MqttClient();

	void setKeepAlive(int seconds);
	// Sets Last Will and Testament
	bool setWill(String topic, String message, int QoS, bool retained = false);

	bool connect(String clientName);
	bool connect(String clientName, String username, String password);

	__forceinline bool isProcessing()  { return TcpClient::isProcessing(); }
	__forceinline TcpClientState getConnectionState() { return TcpClient::getConnectionState(); }

	bool publish(String topic, String message, bool retained = false);
	bool publishWithQoS(String topic, String message, int QoS, bool retained = false);

	bool subscribe(String topic);
	bool unsubscribe(String topic);

protected:
	virtual err_t onReceive(pbuf *buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	void debugPrintResponseType(int type, int len);
	static int staticSendPacket(void* userInfo, const void* buf, unsigned int count);

private:
	String server;
	IPAddress serverIp;
	int port;
	mqtt_broker_handle_t broker;
	int waitingSize;
	uint8_t buffer[MQTT_MAX_BUFFER_SIZE + 1];
	uint8_t *current;
	int posHeader;
	MqttStringSubscriptionCallback callback;
	int keepAlive = 20;

};

#endif /* _SMING_CORE_NETWORK_MqttClient_H_ */
