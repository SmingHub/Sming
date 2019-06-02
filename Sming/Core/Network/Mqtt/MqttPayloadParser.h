/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MqttPayloadParser.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "Delegate.h"
#include "mqtt-codec/src/message.h"

/** @defgroup   mqttpayload Provides MQTT payload parser
 *  @brief      MQTT streaming processor for the payload data of a PUBLISH message
 *  @ingroup    mqtt
 *  @{
 */

#define MQTT_PAYLOAD_PARSER_START -1
#define MQTT_PAYLOAD_PARSER_END -2

#define MQTT_PAYLOAD_LENGTH 1024

typedef struct {
	void* userData; ///< custom user data
	size_t offset;  ///< bytes read so far.
} MqttPayloadParserState;

/**
 * A payload parser must return 0 on success
 */
typedef Delegate<int(MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer, int length)>
	MqttPayloadParser;

int defaultPayloadParser(MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer, int length);

/** @} */
