/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Query.cpp
 *
 ****/

#include "include/Network/Mdns/Query.h"
#include "Packet.h"

namespace mDNS
{
uint16_t Query::serialize(uint8_t* buffer) const
{
	if(name.length() > Name::maxLength - 1) {
		debug_e("Query name too long: '%s'", name.c_str());
		return 0;
	}

	Packet pkt{buffer};
	size_t pos{0};
	auto namelen = name.length();
	do {
		int sep = name.indexOf('.', pos);
		auto wordLength = (sep >= 0) ? (sep - pos) : (namelen - pos);
		pkt.write8(wordLength);
		pkt.write(name.c_str() + pos, wordLength);
		pos = sep + 1;
	} while(pos > 0);
	pkt.write8(0); // End of name.

	// 2 bytes for type
	pkt.write16(uint16_t(type));

	// 2 bytes for class
	pkt.write16(qclass);

	return pkt.pos;
}

size_t serialize(const Query::List& list, uint8_t* buffer, size_t bufSize)
{
	Packet pkt{buffer};

	// The first two bytes are the transaction id and they are not used in MDNS
	pkt.write16(0);

	// 2 bytes for Flags and status code
	pkt.write16(0); // 0x0000 for Query, 0x8000 for Answer

	// 2 bytes for number of questions
	pkt.write16(list.count());

	// 2 bytes for number of Answer RRs
	pkt.write16(0);

	// 2 bytes for Authority PRs
	pkt.write16(0);

	// 2 bytes for Additional PRs
	pkt.write16(0);

	for(auto& query : list) {
		auto len = query.serialize(pkt.ptr());
		if(len == 0) {
			return 0;
		}
		pkt.pos += len;
	}

	return pkt.pos;
}

} // namespace mDNS