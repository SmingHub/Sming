/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Question.cpp
 *
 ****/

#include "include/Network/Mdns/Question.h"
#include "include/Network/Mdns/Message.h"
#include "Packet.h"
#include <debug_progmem.h>

namespace mDNS
{
uint16_t Question::getClass() const
{
	auto qclass = Packet{message.resolvePointer(namePtr + nameLen + 2)}.read16();
	return qclass & 0x7fff;
}

bool Question::isUnicastReply() const
{
	auto qclass = Packet{message.resolvePointer(namePtr + nameLen + 2)}.read16();
	return qclass & 0x8000;
}

Resource::Type Question::getType() const
{
	auto type = Packet{message.resolvePointer(namePtr + nameLen)}.read16();
	return Resource::Type(type);
}

bool Question::parse(Packet& pkt)
{
	auto size = message.getSize();

	namePtr = pkt.pos;
	nameLen = getName().getDataLength();
	pkt.skip(nameLen + 4);

	if(pkt.pos > size) {
		debug_e("[MDNS] Question packet overrun, pos = %u, size = %u", pkt.pos, size);
		// Something has gone wrong receiving or parsing the data.
		return false;
	}

	return true;
}

uint16_t Question::init(uint16_t namePtr, const String& name, ResourceType type, uint16_t qclass, bool unicast)
{
	this->namePtr = namePtr;
	nameLen = message.writeName(namePtr, name);
	Packet pkt{message.resolvePointer(namePtr), nameLen};
	pkt.write16(uint16_t(type));
	qclass &= 0x7fff;
	if(unicast) {
		qclass |= 0x8000;
	}
	pkt.write16(qclass);
	return pkt.pos;
}

} // namespace mDNS
