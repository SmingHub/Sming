/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPointImpl.h - Esp8266 WiFi Access Point
 *
 ****/

#pragma once

#include <Platform/AccessPoint.h>
#include <Platform/System.h>

class AccessPointImpl : public AccessPointClass, protected ISystemReadyHandler
{
public:
	AccessPointImpl()
	{
		System.onReady(this);
	}

	void enable(bool enabled, bool save) override;
	bool isEnabled() const override;
	bool config(const String& ssid, String password, AUTH_MODE mode, bool hidden, int channel,
				int beaconInterval) override;
	IPAddress getIP() const override;
	bool setIP(IPAddress address) override;
	MACAddress getMacAddr() const override;
	IPAddress getNetworkMask() const override;
	IPAddress getNetworkGateway() const override;
	IPAddress getNetworkBroadcast() const override;
	String getSSID() const override;
	String getPassword() const override;

protected:
	void onSystemReady() override;

private:
	softap_config* runConfig = nullptr;
};
