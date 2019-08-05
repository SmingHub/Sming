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

/* StationClass */

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

bool StationClass::setIP(IPAddress address)
{
	IPAddress mask = IPAddress(255, 255, 255, 0);
	IPAddress gateway = IPAddress(address);
	gateway[3] = 1; // x.x.x.1
	return setIP(address, mask, gateway);
}

String StationClass::getMAC(char sep) const
{
	auto mac = getMacAddr();
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
