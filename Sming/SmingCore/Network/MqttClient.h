/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   mqttclient MQTT client
 *  @brief      Provides MQTT client
 *  @ingroup    tcpclient
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_MqttClient_H_
#define _SMING_CORE_NETWORK_MqttClient_H_

#define MQTT_MAX_BUFFER_SIZE 1024

#include "TcpClient.h"
#include "Delegate.h"
#include "WString.h"
#include "WHashMap.h"
#include "../Services/libemqtt/libemqtt.h"
#include "Network/URL.h"

//typedef void (*MqttStringSubscriptionCallback)(String topic, String message);
typedef Delegate<void(String topic, String message)> MqttStringSubscriptionCallback;
typedef Delegate<void(uint16_t msgId, int type)> MqttMessageDeliveredCallback;

class MqttClient;
class URL;

class MqttClient : protected TcpClient {
public:
	MqttClient(bool autoDestruct = false);

	/** @brief  Construct an MQTT client 
	*  @deprecated Use instead the empty contructor
	*/
	MqttClient(String serverHost, int serverPort, MqttStringSubscriptionCallback callback = nullptr);
	/** @brief  Construct an MQTT client
	*  @deprecated Use instead the empty contructor
	*/
	MqttClient(IPAddress serverIp, int serverPort, MqttStringSubscriptionCallback callback = nullptr);
	virtual ~MqttClient();

	/** @brief  Provide a funcion to be called when a message is received from the broker
	*/
	void setCallback(MqttStringSubscriptionCallback subscriptionCallback = nullptr);

	void setKeepAlive(int seconds);		 //send to broker
	void setPingRepeatTime(int seconds); //used by client
	// Sets Last Will and Testament
	bool setWill(const String& topic, const String& message, int QoS, bool retained = false);

	/** @brief  Connect to a MQTT server
	*  @param  url, in the form "mqtt://user:password@server:port" or "mqtts://user:password@server:port"
	*  @param  client name
	*/
	bool connect(const URL& url, const String& uniqueClientName, uint32_t sslOptions = 0);

	/** @brief  connect
	*  @deprecated Use connect(const String& url, const String& uniqueClientName) instead
	*/
	bool connect(const String& clientName, boolean useSsl = false, uint32_t sslOptions = 0);
	/** @brief  connect
	*  @deprecated Use connect(const String& url, const String& uniqueClientName) instead
	*/
	bool connect(const String& clientName, const String& username, const String& password, boolean useSsl = false,
				 uint32_t sslOptions = 0);

	using TcpClient::setCompleteDelegate;
	using TcpClient::isProcessing;
	using TcpClient::getConnectionState;

	bool publish(String topic, String message, bool retained = false);
	bool publishWithQoS(String topic, String message, int QoS, bool retained = false,
						MqttMessageDeliveredCallback onDelivery = nullptr);

	bool subscribe(const String& topic);
	bool unsubscribe(const String& topic);

#ifdef ENABLE_SSL
	using TcpClient::addSslOptions;
	using TcpClient::addSslValidator;
	using TcpClient::freeSslKeyCert;
	using TcpClient::getSsl;
	using TcpClient::pinCertificate;
	using TcpClient::setSslKeyCert;
#endif

protected:
	virtual err_t onReceive(pbuf* buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	void debugPrintResponseType(unsigned type, unsigned len);
	static int staticSendPacket(void* userInfo, const void* buf, unsigned count);

private:
	bool privateConnect(const String& clientName, const String& username, const String& password,
						boolean useSsl = false, uint32_t sslOptions = 0);

	URL _url;
	mqtt_broker_handle_t _broker = { 0 };
	unsigned _waitingSize = 0;
	uint8_t _buffer[MQTT_MAX_BUFFER_SIZE + 1] = {0};
	uint8_t* _current = nullptr;
	unsigned _posHeader = 0;
	MqttStringSubscriptionCallback _callback = nullptr;
	uint32_t _keepAlive = 60;
	uint32_t _pingRepeatTime = 20;
	uint32_t _lastMessage = 0;
	HashMap<uint16_t, MqttMessageDeliveredCallback> _onDeliveryQueue; ///< Maps message IDs to callbacks
};

/** @} */
#endif /* _SMING_CORE_NETWORK_MqttClient_H_ */
