/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Resource.cpp
 *
 ****/

#include "include/Network/Mdns/Resource.h"
#include "include/Network/Mdns/Answer.h"
#include "Packet.h"
#include <Data/HexString.h>

String toString(mDNS::ResourceType type)
{
	switch(type) {
#define XX(name, value, desc)                                                                                          \
	case mDNS::ResourceType::name:                                                                                     \
		return F(#name);
		MDNS_RESOURCE_TYPE_MAP(XX)
#undef XX
	default:
		return String(unsigned(type));
	}
}

namespace mDNS
{
namespace Resource
{
/* Record */

uint8_t* Record::getRecord() const
{
	return answer.getRecord();
}

uint16_t Record::getRecordSize() const
{
	return answer.getRecordSize();
}

String Record::toString() const
{
	return makeHexString(getRecord(), getRecordSize(), ' ');
}

/* A */

IpAddress A::getAddress() const
{
	// Keep bytes in network order
	uint32_t addr;
	memcpy(&addr, getRecord(), sizeof(addr));
	return addr;
}

void A::init(IpAddress ipaddr)
{
	uint32_t addr = ipaddr;
	Packet pkt{getRecord()};
	pkt.write(&addr, sizeof(addr));
	answer.allocate(pkt.pos);
}

/* PTR */

Name PTR::getName() const
{
	return Name(answer.getResponse(), answer.getRecordPtr());
}

void PTR::init(const String& name)
{
	Packet pkt{getRecord()};
	pkt.writeName(name);
	answer.allocate(pkt.pos);
}

/* TXT */

String TXT::toString(const String& sep) const
{
	String s;
	Packet pkt{getRecord()};
	auto size = getRecordSize();
	while(pkt.pos < size) {
		auto len = pkt.read8();
		if(s) {
			s += sep;
		}
		s += pkt.readString(len);
	}
	return s;
}

uint8_t TXT::count() const
{
	if(mCount == 0) {
		Packet pkt{getRecord()};
		auto size = getRecordSize();
		while(pkt.pos < size) {
			auto len = pkt.read8();
			pkt.skip(len);
			++mCount;
		}
	}
	return mCount;
}

String TXT::operator[](uint8_t index) const
{
	uint8_t len;
	auto p = get(index, len);
	return p ? String(p, len) : nullptr;
}

String TXT::getValue(const char* name, uint16_t namelen) const
{
	Packet pkt{getRecord()};
	auto size = getRecordSize();
	while(pkt.pos < size) {
		auto len = pkt.read8();
		auto entry = reinterpret_cast<const char*>(pkt.ptr());
		if(len > namelen && entry[namelen] == '=' && memicmp(entry, name, namelen) == 0) {
			return String(entry + namelen + 1, len - namelen - 1);
		}
		pkt.skip(len);
	}
	return nullptr;
}

const char* TXT::get(uint8_t index, uint8_t& len) const
{
	Packet pkt{getRecord()};
	auto size = getRecordSize();
	for(; pkt.pos < size; --index) {
		len = pkt.read8();
		if(index == 0) {
			return reinterpret_cast<const char*>(pkt.ptr());
		}
		pkt.skip(len);
	}
	return nullptr;
}

void TXT::add(const String& value)
{
	Packet pkt{getRecord(), getRecordSize()};
	auto len = value.length();
	pkt.write8(len);
	pkt.write(value.c_str(), len);
	answer.allocate(pkt.pos);
}

/* AAAA */

String AAAA::toString() const
{
	return makeHexString(getRecord(), getRecordSize(), ':');
}

void AAAA::init(Ip6Address addr)
{
	Packet pkt{getRecord()};
	pkt.write(&addr, sizeof(addr));
	answer.allocate(pkt.pos);
}

/* SRV */

uint16_t SRV::getPriority() const
{
	return Packet{getRecord()}.read16();
}

uint16_t SRV::getWeight() const
{
	return Packet{getRecord(), 2}.read16();
}

uint16_t SRV::getPort() const
{
	return Packet{getRecord(), 4}.read16();
}

Name SRV::getHost() const
{
	return Name(answer.getResponse(), answer.getRecordPtr() + 6);
}

String SRV::toString(const String& sep) const
{
	String s;
	s.reserve(32);
	s += "p=";
	s += getPriority();
	s += sep;
	s += "w=";
	s += getWeight();
	s += sep;
	s += F("port=");
	s += getPort();
	s += sep;
	s += F("host=");
	s += getHost();
	return s;
}

void SRV::init(uint16_t priority, uint16_t weight, uint16_t port, const String& host)
{
	Packet pkt{getRecord()};
	pkt.write16(priority);
	pkt.write16(weight);
	pkt.write16(port);
	pkt.writeName(host);
	answer.allocate(pkt.pos);
}

} // namespace Resource
} // namespace mDNS
