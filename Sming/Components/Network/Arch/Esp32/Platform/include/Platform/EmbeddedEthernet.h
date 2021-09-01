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
 * 
 * See https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/network/esp_eth.html.
 *
 * These are the reduced (RMII) PHY connections.
 * Note that the high-speed signals may not be re-allocated.
 * 
 * 		Signal	  GPIO  Direction
 * 		------	  ----  ---------
 * 		TXD0      19    OUT
 * 		TXD1      22    OUT
 * 		TX_EN     21    OUT
 * 		RXD0      25    IN
 * 		RXD1      26    IN
 * 		RX_DV     27    IN
 *        Receive Data Valid
 * 		MDC       23    OUT (set via MacConfig)
 * 		MDIO      18    OUT (set via MacConfig)
 * 		CLK_MII   0     IN or OUT
 *        50MHz clock provided either by the PHY or the MAC. Default is PHY.
 *
 * The following connections are optional and can set to PIN_UNUSED.
 *
 * 		PHY_RESET 5   OUT (set via PhyConfig)
 */
class EmbeddedEthernet : public Ethernet::Service
{
public:
	bool begin(const Ethernet::MacConfig& macConfig, Ethernet::PhyFactory* phyFactory,
			   const Ethernet::PhyConfig& phyConfig) override;
	void end() override;
	MacAddress getMacAddress() const override;
	bool setMacAddress(const MacAddress& addr) override;
	bool setSpeed(Ethernet::Speed speed) override;
	bool setFullDuplex(bool enable) override;
	bool setLinkState(bool up) override;
	bool setPromiscuous(bool enable) override;

private:
	void enableEventCallback(bool enable);
	void enableGotIpCallback(bool enable);

	void* handle{nullptr};
	esp_eth_mac_s* mac{nullptr};
	esp_eth_phy_s* phy{nullptr};
	Ethernet::Event state{Ethernet::Event::Disconnected};
};
