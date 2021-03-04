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

namespace mDNS
{
Server server;

Server::~Server()
{
	if(active) {
		UdpConnection::leaveMulticastGroup(MDNS_IP);
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

	begin();
	out.listen(0);
	return out.sendTo(request.getRemoteIp(), request.getRemotePort(), buf, len);
}

bool Server::begin()
{
	if(active) {
		return true;
	}

	auto localIp = WifiStation.getIP();

	if(!joinMulticastGroup(localIp, MDNS_IP)) {
		debug_w("[mDNS] joinMulticastGroup() failed");
		return false;
	}

	if(!listen(MDNS_SOURCE_PORT)) {
		debug_e("[mDNS] listen failed");
		return false;
	}

	setMulticast(localIp);
	setMulticastTtl(MDNS_TTL);

	active = true;
	return true;
}

void Server::end()
{
	if(!active) {
		return;
	}

	close();
	leaveMulticastGroup(MDNS_IP);
	active = false;
}

void Server::UdpOut::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	server.onReceive(buf, remoteIP, remotePort);
}

void Server::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	if(packetCallback) {
		packetCallback(remoteIP, remotePort, static_cast<const uint8_t*>(buf->payload), buf->len);
	}

	if(!messageCallback) {
		return;
	}

	Message message(remoteIP, remotePort, buf->payload, buf->len);
	if(message.parse()) {
		messageCallback(message);
	}
}

} // namespace mDNS
