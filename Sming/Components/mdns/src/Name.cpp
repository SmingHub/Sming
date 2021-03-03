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
#include "include/Network/Mdns/Response.h"
#include "Packet.h"

namespace mDNS
{
Name::Info Name::parse() const
{
	Info info{};
	Packet pkt{response.resolvePointer(ptr)};
	while(true) {
		if(pkt.peek8() < 0xC0) {
			++info.components;
			auto wordLen = pkt.read8();
			info.textLength += wordLen;
			pkt.skip(wordLen);
			if(pkt.peek8() == 0) {
				if(info.dataLength == 0) {
					info.dataLength = pkt.pos + 1;
				}
				return info;
			}
			++info.textLength; // separator
		} else {
			uint16_t pointer = pkt.read16() & 0x3fff;
			if(info.dataLength == 0) {
				info.dataLength = pkt.pos;
			}
			pkt = Packet{response.resolvePointer(pointer)};
		}
	}
}

uint16_t Name::read(char* buffer, uint16_t bufSize, uint8_t firstElement, uint8_t count) const
{
	uint16_t pos{0};
	Packet pkt{response.resolvePointer(ptr)};
	while(count != 0) {
		if(pkt.peek8() < 0xC0) {
			auto wordLen = pkt.read8();
			if(firstElement == 0) {
				--count;
				if(pos + wordLen > bufSize) {
					break;
				}
				pkt.read(&buffer[pos], wordLen);
				pos += wordLen;
			} else {
				--firstElement;
				pkt.skip(wordLen);
			}
			if(count == 0 || pkt.peek8() == 0) {
				break;
			}
			if(pos >= bufSize) {
				break;
			}
			if(pos != 0) {
				buffer[pos++] = '.';
			}
		} else {
			uint16_t pointer = pkt.read16() & 0x3fff;
			pkt = Packet{response.resolvePointer(pointer)};
		}
	}
	return pos;
}

String Name::getString(uint8_t firstElement, uint8_t count) const
{
	char buffer[maxLength];
	auto len = read(buffer, maxLength, firstElement, count);
	return String(buffer, len);
}

String Name::getDomain() const
{
	auto info = parse();
	return getString(info.components - 1, 1);
}

String Name::getService() const
{
	auto info = parse();
	return getString(info.components - 2, 1);
}

String Name::getInstance() const
{
	auto info = parse();
	return getString(0, info.components - 2);
}

} // namespace mDNS
