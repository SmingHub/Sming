/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HostPacket.h
 *
 ****/

#pragma once

#include "Packet.h"

/**
 * @brief Helper class for reading/writing packet content in host byte-order (LSB first)
 */
struct HostPacket : public Packet {
	using Packet::Packet;

	uint16_t peek16() const
	{
		return data[pos] | (data[pos + 1] << 8);
	}

	uint16_t read16() const
	{
		return read8() | (read8() << 8);
	}

	uint32_t read32() const
	{
		return read16() | (read16() << 16);
	}

	void write16(uint16_t value)
	{
		write8(value & 0xff);
		write8(value >> 8);
	}

	void write32(uint32_t value)
	{
		write16(value & 0xffff);
		write16(value >> 16);
	}
};
