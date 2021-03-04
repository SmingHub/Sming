/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Server.cpp
 *
 ****/

#include "include/Network/Mdns/Server.h"
#include "Packet.h"
#include <Platform/Station.h>

#define MDNS_IP 224, 0, 0, 251
#define MDNS_TARGET_PORT 5353
#define MDNS_SOURCE_PORT 5353
#define MDNS_TTL 255

#define MAX_PACKET_SIZE 1024

namespace mDNS
{
Server::~Server()
{
	if(initialised) {
		UdpConnection::leaveMulticastGroup(IpAddress(MDNS_IP));
	}
}

bool Server::search(const String& name, ResourceType type)
{
	Request req(Request::Type::query);
	auto question = req.addQuestion(name, type);
	return send(req);
}

bool Server::send(Request& request)
{
	auto buf = reinterpret_cast<const char*>(request.getData());
	auto len = request.getSize();

	initialise();
	out.listen(0);
	return out.sendTo(IpAddress(MDNS_IP), MDNS_TARGET_PORT, buf, len);
}

bool Server::initialise()
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

void Server::UdpOut::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	finder.onReceive(buf, remoteIP, remotePort);
}

void Server::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
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
