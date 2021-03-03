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

bool Finder::search(const String& name, ResourceType type)
{
	Query query{name, type};
	return search(query);
}

bool Finder::search(const Query& query)
{
	Query::List list;
	list.add(&query);
	return search(list);
}

bool Finder::search(const Query::List& queries)
{
	uint8_t buffer[MAX_PACKET_SIZE];
	auto len = serialize(queries, buffer, sizeof(buffer));
	if(len == 0) {
		return false;
	}

	initialise();
	out.listen(0);
	return out.sendTo(IpAddress(MDNS_IP), MDNS_TARGET_PORT, reinterpret_cast<const char*>(buffer), len);
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
	if(packetCallback) {
		packetCallback(remoteIP, remotePort, static_cast<const uint8_t*>(buf->payload), buf->len);
	}

	if(!answerCallback) {
		return;
	}

	Response response(remoteIP, remotePort, buf->payload, buf->len);
	if(response.parse()) {
		answerCallback(response);
	}
}

} // namespace mDNS
