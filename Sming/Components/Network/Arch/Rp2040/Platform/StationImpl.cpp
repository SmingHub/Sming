/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationImpl.cpp
 *
 ****/

#include "StationImpl.h"
#include "WifiEventsImpl.h"

static StationImpl station;
StationClass& WifiStation = station;

void StationImpl::enable(bool enabled, bool save)
{
}

bool StationImpl::isEnabled() const
{
	return false;
}

bool StationImpl::config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save)
{
	return false;
}

bool StationImpl::connect()
{
	return false;
}

bool StationImpl::disconnect()
{
	return false;
}

bool StationImpl::isEnabledDHCP() const
{
	return false;
}

void StationImpl::enableDHCP(bool enable)
{
}

void StationImpl::setHostname(const String& hostname)
{
}

String StationImpl::getHostname() const
{
	return nullptr;
}

IpAddress StationImpl::getIP() const
{
	return IpAddress{};
}

MacAddress StationImpl::getMacAddress() const
{
	return MacAddress{};
}

bool StationImpl::setMacAddress(const MacAddress& addr) const
{
	return false;
}

IpAddress StationImpl::getNetworkBroadcast() const
{
	return IpAddress{};
}

IpAddress StationImpl::getNetworkMask() const
{
	return IpAddress{};
}

IpAddress StationImpl::getNetworkGateway() const
{
	return IpAddress{};
}

bool StationImpl::setIP(IpAddress address, IpAddress netmask, IpAddress gateway)
{
	return false;
}

String StationImpl::getSSID() const
{
	return nullptr;
}

int8_t StationImpl::getRssi() const
{
	return 0;
}

uint8_t StationImpl::getChannel() const
{
	return 0;
}

String StationImpl::getPassword() const
{
	return nullptr;
}

StationConnectionStatus StationImpl::getConnectionStatus() const
{
	return eSCS_Idle;
}

bool StationImpl::startScan(ScanCompletedDelegate scanCompleted)
{
	return false;
}

void StationImpl::onSystemReady()
{
}

#ifdef ENABLE_SMART_CONFIG

void StationImpl::internalSmartConfig(sc_status status, void* pdata)
{
}

bool StationImpl::smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback)
{
	return false;
}

void StationImpl::smartConfigStop()
{
}

#endif // ENABLE_SMART_CONFIG

#ifdef ENABLE_WPS

bool StationImpl::wpsConfigure(uint8_t credIndex)
{
	return false;
}

bool StationImpl::wpsConfigStart(WPSConfigDelegate callback)
{
	return false;
}

void StationImpl::wpsConfigStop()
{
}

#endif // ENABLE_WPS
