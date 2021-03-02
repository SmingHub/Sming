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

	namePtr = pkt.data;
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
		isValid = false;
		return false;
	}

	uint16_t rdlength = pkt.read16();

	rawData = pkt.ptr();
	rawDataLen = rdlength;

	switch(type) {
	case ResourceType::A: // Returns a 32-bit IPv4 address
		a.addr = pkt.read32();
		data = IpAddress(a.addr).toString();
		break;

	case ResourceType::PTR: // Pointer to a canonical name.
		data = Name(response, pkt.data);
		break;

	case ResourceType::HINFO: // HINFO. host information
		data = pkt.readString(rdlength);
		break;

	case ResourceType::TXT: // Originally for arbitrary human-readable text in a DNS record.
		// We only return the first MAX_MDNS_NAME_LEN bytes of this record type.
		data = pkt.readString(rdlength);
		break;

	case ResourceType::AAAA: { // Returns a 128-bit IPv6 address.
		data = makeHexString(static_cast<const uint8_t*>(pkt.ptr()), rdlength, ':');
		break;
	}

	case ResourceType::SRV: { // Server Selection.
		srv.priority = pkt.read16();
		srv.weight = pkt.read16();
		srv.port = pkt.read16();
		data = Name(response, pkt.data);
		// char buffer[64];
		// sprintf(buffer, "p=%u;w=%u;port=%u;host=", priority, weight, port);
		// answer->data = buffer;
		break;
	}

	default:
		data = makeHexString(static_cast<const uint8_t*>(pkt.ptr()), rdlength, ' ');
	}

	pkt.data = rawData + rawDataLen;
	isValid = true;
	return true;
}
} // namespace mDNS