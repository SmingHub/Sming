/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MqttMessage.h - C++ utilities to simplify handling MQTT messages and buffers
 *
 ****/

#pragma once

#include <mqtt-codec/src/message.h>
#include <Print.h>

/**
 * @brief Helper class to simplify printing and parsing message buffers
 */
class MqttBuffer
{
public:
	MqttBuffer(const mqtt_buffer_t& buf) : buf(buf)
	{
	}

	operator String() const
	{
		return String(reinterpret_cast<const char*>(buf.data), buf.length);
	}

	size_t printTo(Print& p) const
	{
		return p.write(buf.data, buf.length);
	}

private:
	const mqtt_buffer_t& buf;
};
