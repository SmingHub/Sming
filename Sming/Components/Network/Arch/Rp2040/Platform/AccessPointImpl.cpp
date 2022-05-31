/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPointImpl.cpp
 *
 ****/

#include "AccessPointImpl.h"
#include "StationListImpl.h"

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

IpAddress AccessPointImpl::getIP() const
{
	return IpAddress{};
}

IpAddress AccessPointImpl::getNetworkBroadcast() const
{
	return IpAddress{};
}

IpAddress AccessPointImpl::getNetworkMask() const
{
	return IpAddress{};
}

IpAddress AccessPointImpl::getNetworkGateway() const
{
	return IpAddress{};
}

bool AccessPointImpl::setIP(IpAddress address)
{
	return false;
}

MacAddress AccessPointImpl::getMacAddress() const
{
	return MacAddress{};
}

bool AccessPointImpl::setMacAddress(const MacAddress& addr) const
{
	return false;
}

String AccessPointImpl::getSSID() const
{
	return nullptr;
}

String AccessPointImpl::getPassword() const
{
	return nullptr;
}

std::unique_ptr<StationList> AccessPointImpl::getStations() const
{
	return std::unique_ptr<StationList>(new StationListImpl);
}

void AccessPointImpl::onSystemReady()
{
}
