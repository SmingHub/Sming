/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MqttPayloadParser.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "MqttPayloadParser.h"

int defaultPayloadParser(MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer, int length)
{
	if(!message) {
		return -1; // invalid message
	}

	if(length == MQTT_PAYLOAD_PARSER_START) {
		if(message->publish.content.length > MQTT_PAYLOAD_LENGTH) {
			return -2; // this parser does not handle big payloads!
		}

		message->publish.content.data = (uint8_t*)malloc(message->publish.content.length);
		if(!message->publish.content.data) {
			return -3; // not enough memory
		}

		state.offset = 0;
		return 0;
	}

	if(length == MQTT_PAYLOAD_PARSER_END) {
		if(state.offset != message->publish.content.length) {
			debug_e("The payload is not complete?!");
			return -4;
		}
		return 0;
	}

	memcpy(&message->publish.content.data[state.offset], buffer, length);
	state.offset += length;

	return 0;
}
