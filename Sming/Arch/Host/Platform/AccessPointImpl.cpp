/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPoint.cpp
 *
 ****/

#include "AccessPointImpl.h"

static AccessPointImpl accessPoint;
AccessPointClass& WifiAccessPoint = accessPoint;

void AccessPointImpl::enable(bool enabled, bool save)
{
}

bool AccessPointImpl::isEnabled() const
{
	return false;
}

bool AccessPointImpl::config(const String& ssid, String password, WifiAuthMode mode, bool hidden, int channel,
							 int beaconInterval)
{
	return false;
}

IPAddress AccessPointImpl::getIP() const
{
	return INADDR_NONE;
}

IPAddress AccessPointImpl::getNetworkBroadcast() const
{
	return INADDR_NONE;
}

IPAddress AccessPointImpl::getNetworkMask() const
{
	return INADDR_NONE;
}

IPAddress AccessPointImpl::getNetworkGateway() const
{
	return INADDR_NONE;
}

bool AccessPointImpl::setIP(IPAddress address)
{
	return false;
}

MACAddress AccessPointImpl::getMacAddr() const
{
	return MACADDR_NONE;
}

String AccessPointImpl::getSSID() const
{
	return nullptr;
}

String AccessPointImpl::getPassword() const
{
	return nullptr;
}
