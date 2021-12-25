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

class MqttClient;

using MqttDelegate = Delegate<int(MqttClient& client, mqtt_message_t* message)>;
using MqttRequestQueue = ObjectQueue<mqtt_message_t, MQTT_REQUEST_POOL_SIZE>;

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

	/**
	 * @brief Publish a message
	 * @param topic
	 * @param message Message content as String
	 * @param flags Optional flags
	 * @retval bool
	 */
	bool publish(const String& topic, const String& message, uint8_t flags = 0);

	/**
	 * @brief Publish a message
	 * @param topic
	 * @param message Message content as read-only stream
	 * @param flags Optional flags
	 * @retval bool
	 */
	bool publish(const String& topic, IDataSourceStream* stream, uint8_t flags = 0);

	/**
	 * @brief Subscribe to a topic
	 * @param topic
	 * @retval bool
	 */
	bool subscribe(const String& topic);

	/**
	 * @brief Unsubscribe from a topic
	 * @param topic
	 * @retval bool
	 */
	bool unsubscribe(const String& topic);

	/**
	 * @brief Register a callback function to be invoked on incoming event notification
	 * @param type Type of event to be notified of
	 * @param handler The callback. Pass nullptr to cancel callback.
	 */
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
	 * @brief Compute the flags value
	 * @param QoS - Quality of Service
	 * @param retain - Retain flag
	 * @param dup - Duplicate delivery
	 *
	 * @retval uint8_t calculated flags value
	 */
	static uint8_t getFlags(mqtt_qos_t QoS, mqtt_retain_t retain = MQTT_RETAIN_FALSE, mqtt_dup_t dup = MQTT_DUP_FALSE)
	{
		return (retain + (QoS << 1) + (dup << 3));
	}

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

private:
	Url url;

	// callbacks
	using HandlerMap = HashMap<mqtt_type_t, MqttDelegate>;
	HandlerMap eventHandlers;
	MqttPayloadParser payloadParser = nullptr;

	// states
	MqttClientState state = eMCS_Ready;
	MqttPayloadParserState payloadState = {};

	// keep-alive and ping
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
};

/** @} */
