#include "include/Network/Mdns/Answer.h"
#include "include/Network/Mdns/Response.h"
#include "Packet.h"
#include <IpAddress.h>
#include <Data/HexString.h>
#include <debug_progmem.h>

namespace mDNS
{
bool Answer::parse(Packet& pkt)
{
	auto size = response.getSize();

	namePtr = pkt.ptr();
	auto namelen = getName().getDataLength();
	pkt.skip(namelen);

	type = ResourceType(pkt.read16());

	uint8_t rrclass_0 = pkt.read8();
	uint8_t rrclass_1 = pkt.read8();
	isCachedFlush = (0b10000000 & rrclass_0);
	klass = ((rrclass_0 & 0b01111111) << 8) + rrclass_1;

	ttl = pkt.read32();

	if(pkt.pos > size) {
		debug_e("[MDNS] Packet overrun, pos = %u, size = %u", pkt.pos, size);
		// Something has gone wrong receiving or parsing the data.
		return false;
	}

	recordSize = pkt.read16();
	record = pkt.ptr();
	pkt.pos += recordSize;
	return true;
}

String Answer::getRecordString() const
{
	switch(type) {
	case mDNS::ResourceType::A:
		return getA().toString();
	case mDNS::ResourceType::PTR:
		return getPTR().toString();
	case mDNS::ResourceType::HINFO:
		return getHINFO().toString();
	case mDNS::ResourceType::TXT:
		return getTXT().toString();
	case mDNS::ResourceType::AAAA:
		return getAAAA().toString();
	case mDNS::ResourceType::SRV:
		return getSRV().toString();
	default:
		return getRecord().toString();
	}
}

String Answer::Record::toString() const
{
	return makeHexString(answer.record, answer.recordSize, ' ');
}

IpAddress Answer::A::getAddress() const
{
	return Packet{answer.record}.read32();
}

Name Answer::PTR::getName() const
{
	return Name(answer.response, answer.record);
}

String Answer::TXT::toString(const char* sep) const
{
	String s;
	Packet pkt{answer.record, 0};
	while(pkt.pos < answer.recordSize) {
		auto len = pkt.read8();
		if(s) {
			s += sep;
		}
		s += pkt.readString(len);
	}
	return s;
}

uint8_t Answer::TXT::count() const
{
	if(mCount == 0) {
		Packet pkt{answer.record, 0};
		while(pkt.pos < answer.recordSize) {
			auto len = pkt.read8();
			pkt.skip(len);
			++mCount;
		}
	}
	return mCount;
}

String Answer::TXT::operator[](uint8_t index) const
{
	uint8_t len;
	auto p = get(index, len);
	return p ? String(p, len) : nullptr;
}

String Answer::TXT::getValue(const char* name, uint16_t namelen) const
{
	Packet pkt{answer.record, 0};
	while(pkt.pos < answer.recordSize) {
		auto len = pkt.read8();
		auto entry = reinterpret_cast<const char*>(pkt.ptr());
		if(len > namelen && entry[namelen] == '=' && memicmp(entry, name, namelen) == 0) {
			return String(entry + namelen + 1, len - namelen - 1);
		}
		pkt.skip(len);
	}
	return nullptr;
}

const char* Answer::TXT::get(uint8_t index, uint8_t& len) const
{
	Packet pkt{answer.record, 0};
	for(; pkt.pos < answer.recordSize; --index) {
		len = pkt.read8();
		if(index == 0) {
			return reinterpret_cast<const char*>(pkt.ptr());
		}
		pkt.skip(len);
	}
	return nullptr;
}

String Answer::AAAA::toString() const
{
	return makeHexString(answer.record, answer.recordSize, ':');
}

uint16_t Answer::SRV::getPriority() const
{
	return Packet{answer.record}.read16();
}

uint16_t Answer::SRV::getWeight() const
{
	return Packet{answer.record, 2}.read16();
}

uint16_t Answer::SRV::getPort() const
{
	return Packet{answer.record, 4}.read16();
}

Name Answer::SRV::getHost() const
{
	return Name(answer.response, answer.record + 6);
}

String Answer::SRV::toString() const
{
	// char buffer[64];
	// sprintf(buffer, "p=%u;w=%u;port=%u;host=", priority, weight, port);
	// answer->data = buffer;
	return "todo";
}

} // namespace mDNS