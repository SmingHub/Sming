/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPoint.cpp
 *
 ****/

#include "Platform/AccessPoint.h"
#include "Data/HexString.h"

AccessPointClass WifiAccessPoint;

void AccessPointClass::enable(bool enabled, bool save)
{
}

bool AccessPointClass::isEnabled()
{
	return false;
}

bool AccessPointClass::config(const String& ssid, String password, AUTH_MODE mode, bool hidden, int channel,
							  int beaconInterval)
{
	return false;
}

IPAddress AccessPointClass::getIP()
{
	return IPADDR_NONE;
}

IPAddress AccessPointClass::getNetworkBroadcast()
{
	return IPADDR_NONE;
}

IPAddress AccessPointClass::getNetworkMask()
{
	return IPADDR_NONE;
}

IPAddress AccessPointClass::getNetworkGateway()
{
	return IPADDR_NONE;
}

bool AccessPointClass::setIP(IPAddress address)
{
	return false;
}

String AccessPointClass::getMAC(char sep)
{
	return nullptr;
}

String AccessPointClass::getSSID()
{
	return nullptr;
}

String AccessPointClass::getPassword()
{
	return nullptr;
}

void AccessPointClass::onSystemReady()
{
}
