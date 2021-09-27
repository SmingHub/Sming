/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Station.cpp
 *
 ****/

#include "Station.h"

String toString(WpsStatus status)
{
	switch(status) {
#define XX(name)                                                                                                       \
	case WpsStatus::name:                                                                                              \
		return F(#name);
		WPS_STATUS_MAP(XX)
#undef XX
	default:
		return F("Unknown_") + unsigned(status);
	}
}

bool StationClass::isConnected() const
{
	if(getConnectionStatus() != eSCS_GotIP) {
		return false;
	}
	if(getIP().isNull()) {
		return false;
	}

	return true;
}

bool StationClass::isConnectionFailed() const
{
	StationConnectionStatus status = getConnectionStatus();
	return status == eSCS_WrongPassword || status == eSCS_AccessPointNotFound || status == eSCS_ConnectionFailed;
}

bool StationClass::setIP(IpAddress address)
{
	IpAddress mask = IpAddress(255, 255, 255, 0);
	IpAddress gateway = IpAddress(address);
	gateway[3] = 1; // x.x.x.1
	return setIP(address, mask, gateway);
}

String StationClass::getMAC(char sep) const
{
	auto mac = getMacAddress();
	return mac ? mac.toString(sep) : nullptr;
}

String StationClass::getConnectionStatusName() const
{
	switch(getConnectionStatus()) {
	case eSCS_Idle:
		return F("Idle");
	case eSCS_Connecting:
		return F("Connecting");
	case eSCS_WrongPassword:
		return F("Wrong password");
	case eSCS_AccessPointNotFound:
		return F("Access point not found");
	case eSCS_ConnectionFailed:
		return F("Connection failed");
	case eSCS_GotIP:
		return F("Successful connected");
	default:
		SYSTEM_ERROR("Unknown status: %d", getConnectionStatus());
		return nullptr;
	};
}
