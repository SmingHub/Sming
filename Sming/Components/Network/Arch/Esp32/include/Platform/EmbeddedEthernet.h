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

#include "IdfService.h"

struct esp_eth_mac_s;
struct esp_eth_phy_s;
struct esp_netif_obj;

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
 * 		CRS_DV    27    IN
 *        Receive Data Valid
 * 		MDC       23    OUT: configurable
 * 		MDIO      18    OUT: configurable
 * 		CLK_MII   0     IN:  No other pins supported
 *                      OUT: alternate pins: 16, 17
 *        50MHz clock provided either by the PHY or the MAC. Default is PHY.
 *
 * Note: Configuring clock options must be done via SDK (make sdk-menuconfig).
 * ESP-IDF v4.4 will add the ability to override these in software.
 *
 * The following connections are optional:
 *
 * 		PHY_RESET -     OUT (set via PhyConfig)
 */
class EmbeddedEthernet : public Ethernet::IdfService
{
public:
	struct Config {
		Ethernet::PhyConfig phy;
		int8_t smiMdcPin = Ethernet::PIN_DEFAULT;  //< SMI MDC GPIO number
		int8_t smiMdioPin = Ethernet::PIN_DEFAULT; //< SMI MDIO GPIO number
	};

	using IdfService::IdfService;

	/**
	 * @brief Configure and start the ethernet service
	 * @param config Configuration options
	 *
	 * Applications should expect to receive Start and Connected events following this call.
	 */
	bool begin(const Config& config);
};
