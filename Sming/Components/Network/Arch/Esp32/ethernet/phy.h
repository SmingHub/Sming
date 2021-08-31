/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * phy.h
 * 
 * Internal ethernet PHY support routines
 *
 ****/

#pragma once

#include <Platform/Ethernet.h>
#include <esp_eth_phy.h>

static inline eth_phy_config_t get_esp_config(const Ethernet::PhyConfig& config)
{
	return eth_phy_config_t{
		.phy_addr = config.phyAddr,
		.reset_timeout_ms = config.resetTimeout,
		.autonego_timeout_ms = config.autoNegTimeout,
		.reset_gpio_num = config.resetPin,
	};
}

static inline void esp_phy_destroy(Ethernet::PhyInstance* inst)
{
	auto phy = reinterpret_cast<esp_eth_phy_t*>(inst);
	if(phy != nullptr) {
		phy->del(phy);
	}
}
