/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPointImpl.h
 *
 ****/

#pragma once

#include <Platform/AccessPoint.h>
#include "WifiEventsImpl.h"
#include "dhcpserver.h"
#include <Platform/System.h>

namespace SmingInternal::Network
{
class AccessPointImpl : public AccessPointClass, protected ISystemReadyHandler
{
public:
	AccessPointImpl()
	{
		System.onReady(this);
	}

	void enable(bool enabled, bool save) override;
	bool isEnabled() const override;
	bool config(const String& ssid, String password, WifiAuthMode mode, bool hidden, int channel,
				int beaconInterval) override;
	IpAddress getIP() const override;
	bool setIP(IpAddress address) override;
	MacAddress getMacAddress() const override;
	bool setMacAddress(const MacAddress& addr) const override;
	IpAddress getNetworkMask() const override;
	IpAddress getNetworkGateway() const override;
	IpAddress getNetworkBroadcast() const override;
	String getSSID() const override;
	String getPassword() const override;
	std::unique_ptr<StationList> getStations() const override;

	void eventHandler(EventInfo& info);

protected:
	void onSystemReady() override;

private:
	void dhcps_start();
	void dhcps_stop();

	dhcp_server_t dhcp_server{};
	bool enabled{false};
};

extern AccessPointImpl accessPoint;

} // namespace SmingInternal::Network
