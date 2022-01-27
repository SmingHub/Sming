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
#include <esp_wifi.h>

struct esp_netif_obj;

namespace SmingInternal
{
namespace Network
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

	// Called from WifiEventsImpl
	void eventHandler(esp_event_base_t base, int32_t id, void* data)
	{
	}

protected:
	void onSystemReady() override;

private:
	esp_netif_obj* apNetworkInterface{nullptr};
};

extern AccessPointImpl accessPoint;

} // namespace Network
} // namespace SmingInternal
