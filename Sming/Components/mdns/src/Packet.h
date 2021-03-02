#pragma once

#include <WString.h>

namespace mDNS
{
/**
 * @brief Helper class for reading/writing packet content
 */
struct Packet {
	uint8_t* data;
	mutable uint16_t pos;

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

	void write(const void* s, uint16_t len)
	{
		memcpy(ptr(), s, len);
		pos += len;
	}
};

} // namespace mDNS