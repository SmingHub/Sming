#include "include/Network/Mdns/Answer.h"
#include "include/Network/Mdns/Response.h"
#include "Packet.h"
#include <debug_progmem.h>

namespace mDNS
{
ResourceType Answer::getType() const
{
	return ResourceType(Packet{namePtr, nameLen}.read16());
}

uint16_t Answer::getClass() const
{
	auto rrclass = Packet{namePtr, uint16_t(nameLen + 2)}.read16();
	return rrclass & 0x7fff;
}

bool Answer::isCachedFlush() const
{
	auto rrclass = Packet{namePtr, uint16_t(nameLen + 2)}.read16();
	return rrclass & 0x8000;
}

uint32_t Answer::getTtl() const
{
	return Packet{namePtr, uint16_t(nameLen + 4)}.read32();
}

bool Answer::parse(Packet& pkt)
{
	auto size = response.getSize();

	namePtr = pkt.ptr();
	nameLen = getName().getDataLength();
	pkt.skip(nameLen + 8);

	if(pkt.pos > size) {
		debug_e("[MDNS] Packet overrun, pos = %u, size = %u", pkt.pos, size);
		// Something has gone wrong receiving or parsing the data.
		return false;
	}

	recordSize = pkt.read16();
	pkt.pos += recordSize;
	return true;
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

} // namespace mDNS
