/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ClientInfo.cpp
 *
 ****/

#include "Network/GoogleCast/ClientInfo.h"
#include <Network/Mdns/Server.h>

namespace
{
DEFINE_FSTR(fstrServiceName, "_googlecast._tcp.local")
}

namespace GoogleCast
{
bool ClientInfo::startDiscovery()
{
	mDNS::server.addHandler(*this);
	return mDNS::server.search(fstrServiceName);
}

void ClientInfo::stopDiscovery()
{
	mDNS::server.removeHandler(*this);
}

bool ClientInfo::onMessage(mDNS::Message& message)
{
	// Check if we're interested in this reponse
	if(!message.isReply()) {
		return true;
	}
	auto answer = message[mDNS::ResourceType::PTR];
	if(answer == nullptr) {
		return true;
	}
	if(answer->getName() != fstrServiceName) {
		return true;
	}

	// Extract our required information from the message

	answer = message[mDNS::ResourceType::A];
	if(answer == nullptr) {
		return true;
	}

	mDNS::Resource::A a(*answer);
	auto ipaddr = a.getAddress();

	answer = message[mDNS::ResourceType::TXT];
	if(answer == nullptr) {
		return true;
	}

	mDNS::Resource::TXT txt(*answer);
	auto friendlyName = txt["fn"];

	(*this)[friendlyName] = ipaddr;
	return true;
}

} // namespace GoogleCast
