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
#include <BitManipulations.h>
#include <WString.h>
#include <WHashMap.h>
#include <Data/ObjectQueue.h>
#include <Platform/Timers.h>
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

using MqttDelegate = Delegate<int(MqttClient& client, mqtt_message_t* message)>;
using MqttRequestQueue = ObjectQueue<mqtt_message_t, MQTT_REQUEST_POOL_SIZE>;

#ifndef MQTT_NO_COMPAT
/** @deprecated Use MqttDelegate instead */
using MqttStringSubscriptionCallback = Delegate<void(String topic, String message)>;
/** @deprecated Use MqttDelegate instead */
using MqttMessageDeliveredCallback = Delegate<void(uint16_t msgId, int type)>;
#endif

class MqttClient : protected TcpClient
{
public:
	MqttClient(bool withDefaultPayloadParser = true, bool autoDestruct = false);

	~MqttClient();

	/**
	 * @brief Sets keep-alive time. That information is sent during connection to the server
	 * @param seconds
	 */
	void setKeepAlive(uint16_t seconds) //send to broker
	{
		keepAlive = seconds;
		if(seconds < pingRepeatTime) {
			setPingRepeatTime(seconds);
		}
	}

	/**
	 * Sets the interval in which to ping the remote server if there was no activity
	 * @param seconds
	 */
	void setPingRepeatTime(uint16_t seconds)
	{
		seconds = std::min(keepAlive, seconds);
		if(seconds != pingRepeatTime) {
			pingRepeatTime = seconds;
			pingTimer.reset(seconds);
		}
	}

	/**
	 * Sets last will and testament
	 * @param topic
	 * @param message
	 * @param flags QoS, retain, etc flags
	 * @retval bool
	 */
	bool setWill(const String& topic, const String& message, uint8_t flags = 0);

	/** @brief  Connect to a MQTT server
	*  @param  url URL in the form "mqtt://user:password@server:port" or "mqtts://user:password@server:port"
	*  @param  uniqueClientName
	*  @retval bool
	*/
	bool connect(const Url& url, const String& uniqueClientName);

	bool publish(const String& topic, const String& message, uint8_t flags = 0);
	bool publish(const String& topic, IDataSourceStream* stream, uint8_t flags = 0);

	bool subscribe(const String& topic);
	bool unsubscribe(const String& topic);

	void setEventHandler(mqtt_type_t type, MqttDelegate handler)
	{
		eventHandlers[type] = handler;
	}

	/**
	 * @brief Sets or clears a payload parser (for PUBLISH messages from the server to us)
	 * @note We no longer have size limitation for incoming or outgoing messages
	 *         but in order to prevent running out of memory we have a "sane" payload parser
	 *         that will read up to 1K of payload
	 */
	void setPayloadParser(MqttPayloadParser payloadParser = nullptr)
	{
		this->payloadParser = payloadParser;
	}

	/* [ Convenience methods ] */

	/**
	 * @brief Sets a handler to be called after successful MQTT connection
	 *
	 * @param handler
	 */
	void setConnectedHandler(MqttDelegate handler)
	{
		eventHandlers[MQTT_TYPE_CONNACK] = handler;
	}

	/**
	 * @brief Sets a handler to be called after receiving confirmation from the server
	 * for a published message from the client
	 *
	 * @param handler
	 */
	void setPublishedHandler(MqttDelegate handler)
	{
		eventHandlers[MQTT_TYPE_PUBACK] = handler;
		eventHandlers[MQTT_TYPE_PUBREC] = handler;
	}

	/**
	 * @brief Sets a handler to be called after receiving a PUBLISH message from the server
	 *
	 * @param handler
	 */
	void setMessageHandler(MqttDelegate handler)
	{
		eventHandlers[MQTT_TYPE_PUBLISH] = handler;
	}

	/**
	 * @brief Sets a handler to be called on disconnect from the server
	 *
	 * @param handler
	 */
	void setDisconnectHandler(TcpClientCompleteDelegate handler)
	{
		TcpClient::setCompleteDelegate(handler);
	}

	using TcpClient::getSsl;
	using TcpClient::setSslInitHandler;

	using TcpClient::setCompleteDelegate;

	using TcpClient::getConnectionState;
	using TcpClient::isProcessing;

	using TcpClient::getRemoteIp;
	using TcpClient::getRemotePort;

#ifndef MQTT_NO_COMPAT
	/**
	 * @deprecated: Use setWill(const String& topic, const String& message,uint8_t flags) instead
	 */
	bool setWill(const String& topic, const String& message, int QoS, bool retained = false) SMING_DEPRECATED
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
	 * @deprecated: Use publish(const String& topic, const String& message, uint8_t flags = 0) instead.
	 * 			   If you want to have a callback that should be triggered on successful delivery of messages
	 * 			   then use setEventHandler(MQTT_TYPE_PUBACK, youCallback) instead.
	 */
	bool publishWithQoS(const String& topic, const String& message, int QoS, bool retained = false,
						MqttMessageDeliveredCallback onDelivery = nullptr) SMING_DEPRECATED
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

	/**
	 * @brief  Provide a function to be called when a message is received from the broker
	 * @deprecated: Use setEventHandler(MQTT_TYPE_PUBLISH, MqttDelegate handler) instead.
	*/
	void setCallback(MqttStringSubscriptionCallback subscriptionCallback = nullptr) SMING_DEPRECATED
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
	int onMessageEnd(mqtt_message_t* message);

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
	using HandlerMap = HashMap<mqtt_type_t, MqttDelegate>;
	HandlerMap eventHandlers;
	MqttPayloadParser payloadParser = nullptr;

	// states
	MqttClientState state = eMCS_Ready;
	MqttPayloadParserState payloadState = {};

	// keep-alives and pings
	uint16_t keepAlive = 60;
	uint16_t pingRepeatTime = 20; ///< pingRepeatTime should be <= keepAlive
	OneShotElapseTimer<NanoTime::Seconds> pingTimer;

	// messages
	MqttRequestQueue requestQueue;
	mqtt_message_t connectMessage;
	bool connectQueued = false; ///< True if our connect message needs to be sent
	mqtt_message_t* outgoingMessage = nullptr;
	mqtt_message_t incomingMessage;

	// parsers and serializers
	mqtt_serialiser_t serialiser;
	static const mqtt_parser_callbacks_t callbacks;
	mqtt_parser_t parser;

	// client flags
	uint8_t flags = 0;
	/* 7 8 6 5 4 3 2 1 0
	*                   |
	*				    --- set when connected ...
	*/

#ifndef MQTT_NO_COMPAT
	SMING_DEPRECATED MqttMessageDeliveredCallback onDelivery = nullptr;			   ///< @deprecated
	SMING_DEPRECATED MqttStringSubscriptionCallback subscriptionCallback = nullptr; ///< @deprecated
#endif
};

/** @} */
