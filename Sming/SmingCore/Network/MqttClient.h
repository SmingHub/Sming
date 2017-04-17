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
#include "../../Wiring/WHashMap.h"
#include "../../Services/libemqtt/libemqtt.h"

//typedef void (*MqttStringSubscriptionCallback)(String topic, String message);
typedef Delegate<void(String topic, String message)> MqttStringSubscriptionCallback;
typedef Delegate<void(uint16_t msgId, int type)> MqttMessageDeliveredCallback;

class MqttClient;
class URL;

class MqttClient: protected TcpClient
{
public:
	MqttClient(String serverHost, int serverPort, MqttStringSubscriptionCallback callback = NULL);
	MqttClient(const char * serverHost, int serverPort, MqttStringSubscriptionCallback callback = NULL);
	MqttClient(IPAddress serverIp, int serverPort, MqttStringSubscriptionCallback callback = NULL);

	void setKeepAlive(int seconds);			//send to broker
	void setPingRepeatTime(int seconds);            //used by client
	// Sets Last Will and Testament
	bool setWill(String topic, String message, int QoS, bool retained = false);
	bool setWill(const char * topic, const char * message, int QoS, bool retained = false);

	bool connect(String clientName, boolean useSsl = false, uint32_t sslOptions = 0);
	bool connect(String clientName, String username, String password, boolean useSsl = false, uint32_t sslOptions = 0);
	bool connect(const char * clientName, const char * username, const char * password, boolean useSsl = false, uint32_t sslOptions = 0);

	using TcpClient::setCompleteDelegate;

	__forceinline bool isProcessing()  { return TcpClient::isProcessing(); }
	__forceinline TcpClientState getConnectionState() { return TcpClient::getConnectionState(); }

	bool publish(String topic, String message, bool retained = false);
	bool publish(const char * topic, const char * message, bool retained = false);
	bool publishWithQoS(String topic, String message, int QoS, bool retained = false, MqttMessageDeliveredCallback onDelivery = NULL);
	bool publishWithQoS(const char * topic, const char * message, int QoS, bool retained = false, MqttMessageDeliveredCallback onDelivery = NULL);

	bool subscribe(String topic);
	bool subscribe(const char * topic);
	bool unsubscribe(String topic);
	bool unsubscribe(const char * topic);

#ifdef ENABLE_SSL
	using TcpClient::addSslOptions;
	using TcpClient::setSslFingerprint;
	using TcpClient::pinCertificate;
	using TcpClient::setSslClientKeyCert;
	using TcpClient::freeSslClientKeyCert;
	using TcpClient::getSsl;
#endif

protected:
	virtual err_t onReceive(pbuf *buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	void debugPrintResponseType(int type, int len);
	static int staticSendPacket(void* userInfo, const void* buf, unsigned int count);

private:
	char * server;
	IPAddress serverIp;
	int port;
	mqtt_broker_handle_t broker;
	int waitingSize;
	uint8_t buffer[MQTT_MAX_BUFFER_SIZE + 1];
	uint8_t *current;
	int posHeader;
	MqttStringSubscriptionCallback callback;
	int keepAlive = 60;
	int PingRepeatTime = 20;
	unsigned long lastMessage;
	HashMap<uint16_t, MqttMessageDeliveredCallback> onDeliveryQueue;
};

#endif /* _SMING_CORE_NETWORK_MqttClient_H_ */
