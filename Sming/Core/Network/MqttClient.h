/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MqttClient.h
 *
 ****/

#pragma once

#include "TcpClient.h"
#include "Url.h"
#include "WString.h"
#include "WHashMap.h"
#include "Data/ObjectQueue.h"
#include "Mqtt/MqttPayloadParser.h"
#include "mqtt-codec/src/message.h"
#include "mqtt-codec/src/serialiser.h"
#include "mqtt-codec/src/parser.h"

/** @defgroup   mqttclient MQTT client
 *  @brief      Provides MQTT client
 *  @ingroup    tcpclient
 *  @{
 */

enum MqttClientState { eMCS_Ready = 0, eMCS_SendingData };

#ifndef MQTT_REQUEST_POOL_SIZE
#define MQTT_REQUEST_POOL_SIZE 10
#endif

#define MQTT_CLIENT_CONNECTED bit(1)

#define MQTT_FLAG_RETAINED 1

#ifndef MQTT_NO_COMPAT
#define MQTT_MAX_BUFFER_SIZE MQTT_PAYLOAD_LENGTH ///< @deprecated
#define MQTT_MSG_PUBREC MQTT_TYPE_PUBREC		 ///< @deprecated
#endif

class MqttClient;

typedef std::function<int(MqttClient& client, mqtt_message_t* message)> MqttDelegate;
typedef ObjectQueue<mqtt_message_t, MQTT_REQUEST_POOL_SIZE> MqttRequestQueue;

#ifndef MQTT_NO_COMPAT
/** @deprecated Use MqttDelegate instead */
typedef Delegate<void(String topic, String message)> MqttStringSubscriptionCallback;
/** @deprecated Use MqttDelegate instead */
typedef Delegate<void(uint16_t msgId, int type)> MqttMessageDeliveredCallback;
#endif

class MqttClient : protected TcpClient
{
public:
	MqttClient(bool withDefaultPayloadParser = true, bool autoDestruct = false);

	~MqttClient();

	/**
	 * Sets keep-alive time. That information is sent during connection to the server
	 * @param uint16_t seconds
	 */
	void setKeepAlive(uint16_t seconds) //send to broker
	{
		keepAlive = seconds;
	}

	/**
	 * Sets the interval in which to ping the remote server if there was no activity
	 * @param unsigned int seconds
	 */
	void setPingRepeatTime(unsigned seconds);

	/**
	 * Sets last will and testament
	 * @param const String& topic
	 * @param const String& message
	 * @param uint8_t flags -QoS, retain, etc flags
	 */
	bool setWill(const String& topic, const String& message, uint8_t flags = 0);

	/** @brief  Connect to a MQTT server
	*  @param  url, in the form "mqtt://user:password@server:port" or "mqtts://user:password@server:port"
	*  @param  client name
	*/
	bool connect(const Url& url, const String& uniqueClientName, uint32_t sslOptions = 0);

	bool publish(const String& topic, const String& message, uint8_t flags = 0);
	bool publish(const String& topic, IDataSourceStream* stream, uint8_t flags = 0);

	bool subscribe(const String& topic);
	bool unsubscribe(const String& topic);

	void setEventHandler(mqtt_type_t type, MqttDelegate handler)
	{
		eventHandler[type] = handler;
	}

	/**
	 * Sets or clears a payload parser (for PUBLISH messages from the server to us)
	 * Notice: we no longer have size limitation for incoming or outgoing messages
	 *         but in order to prevent running out of memory we have a "sane" payload parser
	 *         that will read up to 1K of payload
	 */
	void setPayloadParser(MqttPayloadParser payloadParser = 0)
	{
		this->payloadParser = payloadParser;
	}

	/* [ Convenience methods ] */

	/**
	 * Sets a handler to be called after successful MQTT connection
	 *
	 * @param MqttDelegate handler
	 */
	void setConnectedHandler(MqttDelegate handler)
	{
		eventHandler[MQTT_TYPE_CONNACK] = handler;
	}

	/**
	 * Sets a handler to be called after receiving confirmation from the server
	 * for a published message from the client
	 *
	 * @param MqttDelegate handler
	 */
	void setPublishedHandler(MqttDelegate handler)
	{
		eventHandler[MQTT_TYPE_PUBACK] = handler;
		eventHandler[MQTT_TYPE_PUBREC] = handler;
	}

	/**
	 * Sets a handler to be called after receiving a PUBLISH message from the server
	 *
	 * @param MqttDelegate handler
	 */
	void setMessageHandler(MqttDelegate handler)
	{
		eventHandler[MQTT_TYPE_PUBLISH] = handler;
	}

	/**
	 * Sets a handler to be called on disconnect from the server
	 *
	 * @param  TcpClientCompleteDelegate handler
	 */
	void setDisconnectHandler(TcpClientCompleteDelegate handler)
	{
		TcpClient::setCompleteDelegate(handler);
	}

#ifdef ENABLE_SSL
	using TcpClient::addSslOptions;
	using TcpClient::addSslValidator;
	using TcpClient::freeSslKeyCert;
	using TcpClient::getSsl;
	using TcpClient::pinCertificate;
	using TcpClient::setSslKeyCert;
#endif

	// deprecated methods below
	using TcpClient::setCompleteDelegate;

	using TcpClient::getConnectionState;
	using TcpClient::isProcessing;

#ifndef MQTT_NO_COMPAT
	/**
	 * @todo deprecate: Use setWill(const String& topic, const String& message,uint8_t flags) instead
	 */
	bool setWill(const String& topic, const String& message, int QoS, bool retained = false)
	{
		uint8_t flags = (uint8_t)(retained + (QoS << 1));
		return setWill(topic, message, flags);
	}

	/**
	 * @removed
	 * 		bool publish(String& topic, String& message, bool retained = false)
	 * Use publish(const String& topic, const String& message, uint8_t flags = 0) instead.
	 */

	/**
	 * @todo deprecate: Use publish(const String& topic, const String& message, uint8_t flags = 0) instead.
	 * 			   If you want to have a callback that should be triggered on successful delivery of messages
	 * 			   then use setEventHandler(MQTT_TYPE_PUBACK, youCallback) instead.
	 */
	bool publishWithQoS(const String& topic, const String& message, int QoS, bool retained = false,
						MqttMessageDeliveredCallback onDelivery = nullptr)
	{
		if(onDelivery) {
			if(QoS == 1) {
				setEventHandler(MQTT_TYPE_PUBACK, onPuback);
				this->onDelivery = onDelivery;
			} else if(QoS == 2) {
				setEventHandler(MQTT_TYPE_PUBREC, onPuback);
				this->onDelivery = onDelivery;
			} else {
				debug_w("No callback is set for QoS == 0");
			}
		}

		uint8_t flags = (uint8_t)(retained + (QoS << 1));
		return publish(topic, message, flags);
	}

	/** @brief  Provide a function to be called when a message is received from the broker
	 * @todo deprecate: Use setEventHandler(MQTT_TYPE_PUBLISH, MqttDelegate handler) instead.
	*/
	void setCallback(MqttStringSubscriptionCallback subscriptionCallback = nullptr)
	{
		this->subscriptionCallback = subscriptionCallback;
		setEventHandler(MQTT_TYPE_PUBLISH, onPublish);
	}
#endif

protected:
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;
	void onFinished(TcpClientState finishState) override;

private:
	// TCP methods
	virtual bool onTcpReceive(TcpClient& client, char* data, int size);

	// MQTT parser methods
	static int staticOnMessageBegin(void* user_data, mqtt_message_t* message);
	static int staticOnDataBegin(void* user_data, mqtt_message_t* message);
	static int staticOnDataPayload(void* user_data, mqtt_message_t* message, const char* data, size_t length);
	static int staticOnDataEnd(void* user_data, mqtt_message_t* message);
	static int staticOnMessageEnd(void* user_data, mqtt_message_t* message);

#ifndef MQTT_NO_COMPAT
	/** @deprecated This method is only for compatibility with the previous release and will be removed soon. */
	static int onPuback(MqttClient& client, mqtt_message_t* message)
	{
		if(!message) {
			return 1;
		}

		if(client.onDelivery) {
			uint16_t msgId = 0;
			if(message->common.type == MQTT_TYPE_PUBACK) {
				msgId = message->puback.message_id;
			} else if(message->common.type == MQTT_TYPE_PUBREC) {
				msgId = message->pubrec.message_id;
			}

			if(msgId) {
				client.onDelivery(msgId, (int)message->common.type);
			}
		}

		return 0;
	}

	/** @deprecated This method is only for compatibility with the previous release and will be removed soon. */
	static int onPublish(MqttClient& client, mqtt_message_t* message)
	{
		if(message == nullptr) {
			return -1;
		}

		if(message->common.length > MQTT_PAYLOAD_LENGTH) {
			return -2;
		}

		if(client.subscriptionCallback) {
			String topic = String((const char*)message->publish.topic_name.data, message->publish.topic_name.length);
			String content;
			if(message->publish.content.data) {
				content.concat((const char*)message->publish.content.data, message->publish.content.length);
			}
			client.subscriptionCallback(topic, content);
		}

		return 0;
	}
#endif

private:
	Url url;

	// callbacks
	HashMap<mqtt_type_t, MqttDelegate> eventHandler;
	MqttPayloadParser payloadParser = nullptr;

	// states
	MqttClientState state = eMCS_Ready;
	MqttPayloadParserState payloadState;

	// keep-alives and pings
	uint16_t keepAlive = 60;
	unsigned pingRepeatTime = 20;
	unsigned long lastMessage = 0;

	// messages
	MqttRequestQueue requestQueue;
	mqtt_message_t connectMessage;
	mqtt_message_t* outgoingMessage = nullptr;
	mqtt_message_t incomingMessage;

	// parsers and serializers
	static mqtt_serialiser_t serialiser;
	static mqtt_parser_callbacks_t callbacks;
	mqtt_parser_t parser;

	// client flags
	uint8_t flags = 0;
	/* 7 8 6 5 4 3 2 1 0
	*                   |
	*				    --- set when connected ...
	*/

#ifndef MQTT_NO_COMPAT
	MqttMessageDeliveredCallback onDelivery = nullptr;			   ///< @deprecated
	MqttStringSubscriptionCallback subscriptionCallback = nullptr; ///< @deprecated
#endif
};

/** @} */
