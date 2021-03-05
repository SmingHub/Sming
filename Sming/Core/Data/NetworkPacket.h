/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * NetworkPacket.h
 *
 ****/

#pragma once

#include "Packet.h"

/**
 * @brief Helper class for reading/writing packet content in network byte-order (MSB first)
 */
struct NetworkPacket : public Packet {
	using Packet::Packet;

	uint16_t peek16() const
	{
		return (data[pos] << 8) | data[pos + 1];
	}

	uint16_t read16() const
	{
		return (read8() << 8) | read8();
	}

	uint32_t read32() const
	{
		return (read16() << 16) | read16();
	}

	void write16(uint16_t value)
	{
		write8(value >> 8);
		write8(value & 0xff);
	}

	void write32(uint32_t value)
	{
		write16(value >> 16);
		write16(value & 0xffff);
	}
};
