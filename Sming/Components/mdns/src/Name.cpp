#include "include/Network/Mdns/Name.h"
#include "include/Network/Mdns/Response.h"
#include "Packet.h"

namespace mDNS
{
String Name::toString() const
{
	String s;
	Packet pkt{data, 0};

	while(true) {
		if(pkt.peek8() < 0xC0) {
			// Since the first 2 bits are not set,
			// this is the start of a name section.
			// http://www.tcpipguide.com/free/t_DNSNameNotationandMessageCompressionTechnique.htm

			const uint8_t word_len = pkt.read8();
			s += pkt.readString(word_len);

			if(pkt.peek8() == 0) {
				return s; // End of string
			}

			// Next word
			s += '.';
			continue;
		}

		// Message Compression used. Next 2 bytes are a pointer to the actual name section.
		uint16_t pointer = pkt.read16() & 0x3fff;
		pkt.data = response.resolvePointer(pointer);
		pkt.pos = 0;
	}
}

uint16_t Name::getDataLength() const
{
	Packet pkt{data, 0};

	while(true) {
		if(pkt.peek8() < 0xC0) {
			const uint8_t word_len = pkt.read8();
			pkt.skip(word_len);
			if(pkt.peek8() == 0) {
				pkt.read8();
				break;
			}
		} else {
			// Pointer at end
			pkt.read16();
			break;
		}
	}
	return pkt.pos;
}

} // namespace mDNS
