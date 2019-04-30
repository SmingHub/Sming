/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 * Manages GDB packet encoding
 *
 ****/

#include "GdbPacket.h"

// Send the start of a packet; reset checksum calculation.
void ATTR_GDBEXTERNFN GdbPacket::start()
{
#if GDBSTUB_ENABLE_DEBUG
	m_puts(_F("> PKT "));
#endif
	gdbSendChar('$');
}

void ATTR_GDBEXTERNFN GdbPacket::end()
{
	gdbSendChar('#');
	writeHexByte(checksum);
#if GDBSTUB_ENABLE_DEBUG
	m_puts("\r\n");
#endif
}

void ATTR_GDBEXTERNFN GdbPacket::writeChar(char c)
{
	gdbSendChar(c);
	++packetLength;
	checksum += uint8_t(c);
}

void ATTR_GDBEXTERNFN GdbPacket::writeCharEscaped(char c)
{
	if(c == '#' || c == '$' || c == '}' || c == '*') {
		writeChar('}');
		c ^= 0x20;
	}
	writeChar(c);
}

void ATTR_GDBEXTERNFN GdbPacket::writeEscaped(const void* data, unsigned length)
{
	for(unsigned i = 0; i < length; ++i) {
		writeCharEscaped(static_cast<const uint8_t*>(data)[i]);
	}
}

void ATTR_GDBEXTERNFN GdbPacket::write(const void* data, unsigned length)
{
	for(unsigned i = 0; i < length; ++i) {
		checksum += static_cast<const uint8_t*>(data)[i];
	}
	gdbSendData(data, length);
	packetLength += length;
}

void ATTR_GDBEXTERNFN GdbPacket::writeStr(const char* str)
{
	write(str, strlen(str));
}

void ATTR_GDBEXTERNFN GdbPacket::writeStrRef(const char* str)
{
	writeHexWord32(uint32_t(str));
	writeChar('/');
	writeHexWord16(strlen_P(str) + 1);
}

void ATTR_GDBEXTERNFN GdbPacket::writeX32()
{
	for(int i = 0; i < 8; i++) {
		writeChar('x');
	}
}

void ATTR_GDBEXTERNFN GdbPacket::writeHexByte(uint8_t value)
{
	writeChar(hexchar((value >> 4) & 0x0f));
	writeChar(hexchar(value & 0x0f));
}

void ATTR_GDBEXTERNFN GdbPacket::writeHexWord16(uint16_t value)
{
	writeHexByte(value >> 8);
	writeHexByte(value);
}

void ATTR_GDBEXTERNFN GdbPacket::writeHexWord32(uint32_t value)
{
	writeHexByte(value >> 24);
	writeHexByte(value >> 16);
	writeHexByte(value >> 8);
	writeHexByte(value);
}

void ATTR_GDBEXTERNFN GdbPacket::writeHexBlock(const void* src, size_t size)
{
	for(unsigned i = 0; i < size; ++i) {
		writeHexByte(static_cast<const uint8_t*>(src)[i]);
	}
}

uint32_t ATTR_GDBEXTERNFN GdbPacket::readHexValue(const char*& data)
{
	uint32_t result = 0;
	int8_t c;
	while((c = unhex(*data)) >= 0) {
		result = (result << 4) | c;
		++data;
	}
	return result;
}

void ATTR_GDBEXTERNFN GdbPacket::encodeHexBlock(char* dst, const void* src, size_t size)
{
	for(unsigned i = 0; i < size; ++i) {
		uint8_t value = static_cast<const uint8_t*>(src)[i];
		*dst++ = hexchar((value >> 4) & 0x0f);
		*dst++ = hexchar(value & 0x0f);
	}
}

size_t ATTR_GDBEXTERNFN GdbPacket::decodeHexBlock(void* dst, const char*& src)
{
	auto out = static_cast<uint8_t*>(dst);
	for(;;) {
		int8_t c1 = unhex(src[0]);
		if(c1 < 0) {
			break;
		}
		int8_t c0 = unhex(src[1]);
		if(c0 < 0) {
			break;
		}
		src += 2;
		*out++ = (c1 << 4) | c0;
	}
	return out - static_cast<uint8_t*>(dst);
}
