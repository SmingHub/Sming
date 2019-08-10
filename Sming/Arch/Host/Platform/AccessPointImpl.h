/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPointImpl.h - Host WiFi Access Point
 *
 ****/

#pragma once

#include "Platform/AccessPoint.h"

class AccessPointImpl : public AccessPointClass
{
public:
	void enable(bool enabled, bool save) override;
	bool isEnabled() const override;
	bool config(const String& ssid, String password, WifiAuthMode mode, bool hidden, int channel,
				int beaconInterval) override;
	IpAddress getIP() const override;
	bool setIP(IpAddress address) override;
	MacAddress getMacAddress() const override;
	IpAddress getNetworkMask() const override;
	IpAddress getNetworkGateway() const override;
	IpAddress getNetworkBroadcast() const override;
	String getSSID() const override;
	String getPassword() const override;
};
