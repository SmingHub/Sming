/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Name.cpp
 *
 ****/

#include "include/Network/Mdns/Name.h"
#include "include/Network/Mdns/Message.h"
#include "Packet.h"

namespace mDNS
{
static constexpr uint32_t PROTO_TCP{0x7063745F}; // "_tcp"
static constexpr uint32_t PROTO_UDP{0x7064755F}; // "_udp"

/*
 * Class for performing operations on a name in segments
 */
class Reader : public Packet
{
public:
	Reader(const Message& message, uint16_t ptr) : Packet{message.resolvePointer(0), ptr}
	{
	}

	// Return length of next segment, 0 for end of name
	uint8_t next(uint8_t previousLength)
	{
		if(previousLength != 0) {
			skip(previousLength);
		}

		if(peek8() >= 0xC0) {
			pos = read16() & 0x3fff;
		}

		auto len = read8();
		if(len >= 0xC0) {
			debug_e("INVALID @ %u", pos);
			return 0;
		}
		return len;
	}
};

uint16_t Name::getDataLength() const
{
	Packet pkt{message.resolvePointer(ptr)};
	while(true) {
		if(pkt.peek8() >= 0xC0) {
			return pkt.pos + 2;
		}
		auto wordLen = pkt.read8();
		if(wordLen >= 0xC0) {
			// Invalid
			debug_e("INVALID @ %u", pkt.pos - 1);
			return 0;
		}
		if(wordLen == 0) {
			return pkt.pos;
		}
		pkt.skip(wordLen);
	}
}

uint16_t Name::read(char* buffer, uint16_t bufSize) const
{
	uint16_t pos{0};
	Reader reader(message, ptr);
	uint8_t len{0};
	while((len = reader.next(len)) != 0) {
		if(pos == bufSize) {
			break;
		}
		if(pos != 0) {
			buffer[pos++] = '.';
		}
		if(pos + len > bufSize) {
			break;
		}
		memcpy(&buffer[pos], reader.ptr(), len);
		pos += len;
	}
	return pos;
}

String Name::toString() const
{
	char buffer[maxLength];
	auto len = read(buffer, maxLength);
	return String(buffer, len);
}

Name::ElementPointers Name::parseElements() const
{
	bool protocolFound{false};
	ElementPointers elem{};
	Reader reader(message, ptr);
	uint8_t len{0};
	while((len = reader.next(len)) != 0) {
		if(protocolFound) {
			elem.domain = reader.pos - 1;
			return elem;
		}

		elem.service = elem.protocol;
		elem.protocol = reader.pos - 1;

		if(len == 4) {
			uint32_t w;
			memcpy(&w, reader.ptr(), sizeof(w));
			if(w == PROTO_TCP || w == PROTO_UDP) {
				protocolFound = true;
			}
		}
	}

	return ElementPointers{};
}

Name Name::getDomain() const
{
	return Name(message, parseElements().domain);
}

Name Name::getProtocol() const
{
	return Name(message, parseElements().protocol);
}

Name Name::getService() const
{
	return Name(message, parseElements().service);
}

bool Name::equalsIgnoreCase(const char* str, size_t length) const
{
	Reader reader(message, ptr);
	uint8_t len{0};
	while((len = reader.next(len)) != 0) {
		if(len > length) {
			return false;
		}
		auto c = str[len];
		if(c != '.' && c != '\0') {
			return false;
		}
		if(memicmp(str, reader.ptr(), len) != 0) {
			return false;
		}
		if(c == '\0') {
			return len == length;
		}
		str += len + 1;
		length -= len + 1;
	}
	return true;
}

uint16_t Name::makePointer() const
{
	auto content = ptr;
	Packet pkt{message.resolvePointer(content)};
	if(pkt.peek8() >= 0xC0) {
		// Resolve the pointer to data
		content = pkt.read16();
	}
	return content | 0xC000;
}

bool Name::fixup(const Name& other)
{
	auto len = getDataLength();
	if(len < 2) {
		// Can't be a pointer, too small
		return false;
	}
	Packet pkt{message.resolvePointer(ptr + len - 2)};
	if(pkt.peek8() < 0xC0) {
		// Not a pointer
		return false;
	}
	pkt.write16(other.makePointer());
	return true;
}

} // namespace mDNS
