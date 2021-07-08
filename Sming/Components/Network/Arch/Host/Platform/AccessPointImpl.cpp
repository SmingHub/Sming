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

IpAddress AccessPointImpl::getIP() const
{
	return INADDR_NONE;
}

IpAddress AccessPointImpl::getNetworkBroadcast() const
{
	return INADDR_NONE;
}

IpAddress AccessPointImpl::getNetworkMask() const
{
	return INADDR_NONE;
}

IpAddress AccessPointImpl::getNetworkGateway() const
{
	return INADDR_NONE;
}

bool AccessPointImpl::setIP(IpAddress address)
{
	return false;
}

MacAddress AccessPointImpl::getMacAddress() const
{
	return MACADDR_NONE;
}

bool AccessPointImpl::setMacAddress(const MacAddress& addr) const
{
	debug_w("[HOST] setMacAddress not implemented");
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

const AccessPointClass::StationList AccessPointImpl::getStations() const
{
	StationList stationList;
	StationInfo info{};
	info.mac = MacAddress({5, 4, 3, 2, 1, 0});
	info.rssi = 64;
	stationList.addElement(info);

	return stationList;
}
