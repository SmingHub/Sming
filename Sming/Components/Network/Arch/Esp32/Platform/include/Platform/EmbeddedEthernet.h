/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EmbeddedEthernet.h
 *
 ****/

#pragma once

#include <Platform/Ethernet.h>

struct esp_eth_mac_s;
struct esp_eth_phy_s;

/**
 * @brief Ethernet provider using ESP32 embedded MAC. Requires an external PHY.
 */
class EmbeddedEthernet : public Ethernet::Service
{
public:
	bool begin(const Ethernet::MacConfig& config, Ethernet::PhyFactory* phyFactory) override;
	void end() override;

private:
	void enableEventCallback(bool enable);
	void enableGotIpCallback(bool enable);

	void* handle{nullptr};
	esp_eth_mac_s* mac{nullptr};
	esp_eth_phy_s* phy{nullptr};
	Ethernet::Event state{Ethernet::Event::Disconnected};
};
