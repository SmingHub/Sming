/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Finder.cpp
 *
 ****/

#include "include/Network/Mdns/Finder.h"
#include "include/Network/Mdns/Response.h"
#include "Packet.h"
#include <Platform/Station.h>

#define MDNS_IP 224, 0, 0, 251
#define MDNS_TARGET_PORT 5353
#define MDNS_SOURCE_PORT 5353
#define MDNS_TTL 255

#define MAX_PACKET_SIZE 1024

namespace mDNS
{
Finder::~Finder()
{
	if(initialised) {
		UdpConnection::leaveMulticastGroup(IpAddress(MDNS_IP));
	}
}

bool Finder::search(const String& hostname, ResourceType type)
{
	Query query{hostname, type};
	return search(query);
}

bool Finder::search(const Query& query)
{
	if(query.name.length() > MAX_MDNS_NAME_LEN - 1) {
		return false;
	}

	uint8_t buffer[MAX_PACKET_SIZE];
	Packet pkt{buffer};

	// The first two bytes are the transaction id and they are not used in MDNS
	pkt.write16(0);

	// 2 bytes for Flags
	pkt.write8(0); // 0b00000000 for Query, 0b10000000 for Answer.
	pkt.write8(0);

	// 2 bytes for number of questions
	pkt.write16(1);

	// 2 bytes for number of Answer RRs
	pkt.write16(0);

	// 2 bytes for Authority PRs
	pkt.write16(0);

	// 2 bytes for Additional PRs
	pkt.write16(0);

	size_t pos{0};
	auto& name = query.name;
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
	pkt.write16(uint16_t(query.type));

	// 2 bytes for class
	uint16_t qclass = query.klass;
	if(query.isUnicastResponse) {
		qclass |= 0x8000;
	}
	pkt.write16(qclass);

	initialise();
	out.listen(0);
	return out.sendTo(IpAddress(MDNS_IP), MDNS_TARGET_PORT, reinterpret_cast<const char*>(pkt.data), pkt.pos);
}

bool Finder::initialise()
{
	if(initialised) {
		return true;
	}

	auto localIp = WifiStation.getIP();

	if(!joinMulticastGroup(localIp, IpAddress(MDNS_IP))) {
		debug_w("[mDNS] joinMulticastGroup() failed");
		return false;
	}

	if(!listen(MDNS_SOURCE_PORT)) {
		debug_e("[mDNS] listen failed");
		return false;
	}

	setMulticast(localIp);
	setMulticastTtl(MDNS_TTL);

	initialised = true;
	return true;
}

void Finder::UdpOut::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	finder.onReceive(buf, remoteIP, remotePort);
}

void Finder::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	if(!answerCallback) {
		return;
	}

	Response response(remoteIP, remotePort, buf->payload, buf->len);
	if(response.parse()) {
		answerCallback(response);
	}
}

} // namespace mDNS
