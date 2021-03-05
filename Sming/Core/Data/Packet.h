/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Packet.h
 *
 ****/

#pragma once

#include <WString.h>

/**
 * @brief Helper class for reading/writing packet content
 */
struct Packet {
	uint8_t* data;
	mutable uint16_t pos;

	Packet(void* data, uint16_t pos = 0) : data(static_cast<uint8_t*>(data)), pos(pos)
	{
	}

	const uint8_t* ptr() const
	{
		return data + pos;
	}

	uint8_t* ptr()
	{
		return data + pos;
	}

	void skip(uint16_t len) const
	{
		pos += len;
	}

	uint8_t peek8() const
	{
		return data[pos];
	}

	uint8_t read8() const
	{
		return data[pos++];
	}

	void read(void* buffer, uint16_t len) const
	{
		memcpy(buffer, ptr(), len);
		pos += len;
	}

	String readString(uint16_t length) const
	{
		String s;
		if(s.setLength(length)) {
			read(s.begin(), length);
		} else {
			pos += length;
		}
		return s;
	}

	void write8(uint8_t value)
	{
		data[pos++] = value;
	}

	void write(const void* s, uint16_t len)
	{
		memcpy(ptr(), s, len);
		pos += len;
	}
};
