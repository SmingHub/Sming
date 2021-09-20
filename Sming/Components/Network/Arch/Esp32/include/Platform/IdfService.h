/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * IdfService.h
 *
 ****/

#pragma once

#include <Platform/Ethernet.h>

struct esp_eth_mac_s;
struct esp_eth_phy_s;
struct esp_netif_obj;

namespace Ethernet
{
/**
 * @brief Base Ethernet service for IDF SDK
 */
class IdfService : public Service
{
public:
	IdfService(PhyFactory& phyFactory) : phyFactory(phyFactory)
	{
	}

	void end() override;
	MacAddress getMacAddress() const override;
	bool setMacAddress(const MacAddress& addr) override;
	bool setSpeed(Ethernet::Speed speed) override;
	bool setFullDuplex(bool enable) override;
	bool setLinkState(bool up) override;
	bool setPromiscuous(bool enable) override;
	void setHostname(const String& hostname) override;
	String getHostname() const override;
	IpAddress getIP() const override;
	bool setIP(IpAddress address, IpAddress netmask, IpAddress gateway) override;
	bool isEnabledDHCP() const override;
	bool enableDHCP(bool enable) override;

protected:
	void enableEventCallback(bool enable);
	void enableGotIpCallback(bool enable);

	PhyFactory& phyFactory;
	void* handle{nullptr};
	esp_netif_obj* netif{nullptr};
	void* netif_glue{nullptr};
	esp_eth_mac_s* mac{nullptr};
	esp_eth_phy_s* phy{nullptr};
	Event state{Event::Disconnected};
};

} // namespace Ethernet
