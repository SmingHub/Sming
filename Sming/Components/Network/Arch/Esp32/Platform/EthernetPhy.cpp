/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EthernetPhy.cpp
 * 
 * Class implementations for all supported ESP32 Ethernet PHY devices
 *
 ****/

#include <Platform/Ethernet.h>
#include <Network/Ethernet/Rtl8201.h>
#include <Network/Ethernet/Ip101.h>
#include <Network/Ethernet/Lan8720.h>
#include <Network/Ethernet/Dp83848.h>
#include <Network/Ethernet/Ksz8041.h>
#include <esp_eth_phy.h>

#define PHY_IMPL(class_name, func_name)                                                                                \
	Ethernet::PhyInstance* class_name::create()                                                                        \
	{                                                                                                                  \
		auto phy_config = get_esp_config(config);                                                                      \
		auto phy = esp_eth_phy_new_##func_name(&phy_config);                                                           \
		return reinterpret_cast<PhyInstance*>(phy);                                                                    \
	}                                                                                                                  \
                                                                                                                       \
	void class_name::destroy(PhyInstance* inst)                                                                        \
	{                                                                                                                  \
		esp_phy_destroy(inst);                                                                                         \
	}

namespace
{
eth_phy_config_t get_esp_config(const Ethernet::PhyConfig& config)
{
	return eth_phy_config_t{
		.phy_addr = config.phyAddr,
		.reset_timeout_ms = config.resetTimeout,
		.autonego_timeout_ms = config.autoNegTimeout,
		.reset_gpio_num = config.resetPin,
	};
}

void esp_phy_destroy(Ethernet::PhyInstance* inst)
{
	auto phy = reinterpret_cast<esp_eth_phy_t*>(inst);
	if(phy != nullptr) {
		phy->del(phy);
	}
}

} // namespace

namespace Ethernet
{
PHY_IMPL(Ip101, ip101)
PHY_IMPL(Rtl8201, rtl8201)
PHY_IMPL(Lan8720, lan8720)
PHY_IMPL(Dp83848, dp83848)
PHY_IMPL(Ksz8041, ksz8041)

} // namespace Ethernet
