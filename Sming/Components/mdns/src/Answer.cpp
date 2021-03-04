/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Answer.cpp
 *
 ****/

#include "include/Network/Mdns/Answer.h"
#include "include/Network/Mdns/Message.h"
#include "Packet.h"
#include <debug_progmem.h>

String toString(mDNS::Answer::Kind kind)
{
	using Kind = mDNS::Answer::Kind;
	switch(kind) {
	case Kind::answer:
		return F("answer");
	case Kind::name:
		return F("name");
	case Kind::additional:
		return F("additional");
	default:
		assert(false);
		return nullptr;
	}
}

namespace mDNS
{
ResourceType Answer::getType() const
{
	return ResourceType(Packet{message.resolvePointer(namePtr + nameLen)}.read16());
}

uint16_t Answer::getClass() const
{
	auto rrclass = Packet{message.resolvePointer(namePtr + nameLen + 2)}.read16();
	return rrclass & 0x7fff;
}

bool Answer::isCachedFlush() const
{
	auto rrclass = Packet{message.resolvePointer(namePtr + nameLen + 2)}.read16();
	return rrclass & 0x8000;
}

uint32_t Answer::getTtl() const
{
	return Packet{message.resolvePointer(namePtr + nameLen + 4)}.read32();
}

bool Answer::parse(Packet& pkt)
{
	auto size = message.getSize();

	namePtr = pkt.pos;
	nameLen = getName().getDataLength();
	pkt.skip(nameLen + 8);

	if(pkt.pos > size) {
		debug_e("[MDNS] Answer packet overrun, pos = %u, size = %u", pkt.pos, size);
		// Something has gone wrong receiving or parsing the data.
		return false;
	}

	recordSize = pkt.read16();
	pkt.pos += recordSize;
	return true;
}

uint8_t* Answer::getRecord() const
{
	return message.resolvePointer(getRecordPtr());
}

String Answer::getRecordString() const
{
	using namespace Resource;
	switch(getType()) {
	case Type::A:
		return A(*this).toString();
	case Type::PTR:
		return PTR(*this).toString();
	case Type::HINFO:
		return HINFO(*this).toString();
	case Type::TXT:
		return TXT(*this).toString();
	case Type::AAAA:
		return AAAA(*this).toString();
	case Type::SRV:
		return SRV(*this).toString();
	default:
		return Record(*this).toString();
	}
}

uint16_t Answer::init(uint16_t namePtr, const String& name, Resource::Type type, uint16_t rclass, bool flush,
					  uint32_t ttl)
{
	this->namePtr = namePtr;
	Packet pkt{message.resolvePointer(namePtr)};
	pkt.writeName(name);
	nameLen = pkt.pos;
	pkt.write16(uint16_t(type));						   // Type
	pkt.write16((rclass & 0x7fff) | (flush ? 0x8000 : 0)); // Class
	pkt.write32(ttl);									   // TTL
	pkt.write16(0);										   // Resource Record Length
	return pkt.pos;
}

uint16_t Answer::init(uint16_t namePtr, const Name& name, Resource::Type type, uint16_t rclass, bool flush,
					  uint32_t ttl)
{
	this->namePtr = namePtr;
	Packet pkt{message.resolvePointer(namePtr)};
	pkt.write16(name.getPtr() | 0xC000);
	nameLen = pkt.pos;
	pkt.write16(uint16_t(type));						   // Type
	pkt.write16((rclass & 0x7fff) | (flush ? 0x8000 : 0)); // Class
	pkt.write32(ttl);									   // TTL
	pkt.write16(0);										   // Resource Record Length
	return pkt.pos;
}

void Answer::allocate(uint16_t size)
{
	assert(size > recordSize);
	message.allocate(size - recordSize);
	recordSize = size;
	Packet pkt{message.resolvePointer(namePtr + nameLen + 8)};
	pkt.write16(recordSize);
}

} // namespace mDNS
