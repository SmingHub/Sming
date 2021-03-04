/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ClientInfo.h
 *
 ****/

#pragma once

#include <WHashMap.h>
#include <WString.h>
#include <IpAddress.h>
#include <Network/Mdns/Handler.h>

namespace GoogleCast
{
class ClientInfo : public HashMap<String, IpAddress>, public mDNS::Handler
{
public:
	bool startDiscovery();
	void stopDiscovery();

private:
	bool onMessage(mDNS::Message& message);
};

} // namespace GoogleCast
