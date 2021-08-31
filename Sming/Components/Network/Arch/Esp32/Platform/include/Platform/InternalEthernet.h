/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * InternalEthernet.h
 *
 ****/

#pragma once

#include <Platform/Ethernet.h>
#include <memory>

struct esp_eth_mac_s;
struct esp_eth_phy_s;

/**
 * @brief Ethernet provider using ESP32 internal MAC
 */
class InternalEthernet : public Ethernet
{
public:
	bool begin(PhyFactory* phyFactory) override;
	void end() override;

private:
	void enableEventCallback(bool enable);
	void enableGotIpCallback(bool enable);

	std::unique_ptr<PhyFactory> phyFactory;
	void* handle{nullptr};
	esp_eth_mac_s* mac{nullptr};
	esp_eth_phy_s* phy{nullptr};
	EthernetEvent state{EthernetEvent::Disconnected};
};

/** @} */
