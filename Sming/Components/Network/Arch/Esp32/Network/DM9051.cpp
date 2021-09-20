/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DM9051.cpp
 *
 ****/

#include <Network/Ethernet/DM9051.h>
#include "spi_config.h"

namespace Ethernet
{
DM9051PhyFactory DM9051Service::dm9051PhyFactory;

#define CHECK_RET(err)                                                                                                 \
	if(ESP_ERROR_CHECK_WITHOUT_ABORT(err) != ESP_OK) {                                                                 \
		return false;                                                                                                  \
	}

bool DM9051Service::begin(const Config& config)
{
	esp_netif_init();
	esp_event_loop_create_default();

	esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
	netif = esp_netif_new(&netif_cfg);

	// Set default handlers to process TCP/IP stuffs
	CHECK_RET(esp_eth_set_default_handlers(netif));

	// And register our own event handlers
	enableEventCallback(true);
	enableGotIpCallback(true);

	auto spiHost = (config.spiHost < 0) ? DEFAULT_HOST : spi_host_device_t(config.spiHost);

	auto getPin = [](int pin, uint8_t defaultPin) -> int { return (pin < 0) ? defaultPin : pin; };

	gpio_install_isr_service(0);
	spi_device_interface_config_t devcfg = {
		.command_bits = 1,
		.address_bits = 7,
		.mode = 0,
		.clock_speed_hz = int(config.clockSpeed),
		.spics_io_num = getPin(config.chipSelectPin, DEFAULT_PIN_CS),
		.queue_size = 20,
	};
	spi_device_handle_t spi_handle{nullptr};
	CHECK_RET(spi_bus_add_device(spiHost, &devcfg, &spi_handle));

	eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(spi_handle);
	dm9051_config.int_gpio_num = getPin(config.interruptPin, DEFAULT_PIN_INT);
	eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
	mac = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);

	auto phy_cfg = config.phy;
	phy_cfg.resetPin = getPin(phy_cfg.resetPin, DEFAULT_PIN_RESET);
	phy = reinterpret_cast<esp_eth_phy_t*>(phyFactory.create(phy_cfg));
	if(phy == nullptr) {
		debug_e("[ETH] Failed to construct PHY");
		return false;
	}

	esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
	CHECK_RET(esp_eth_driver_install(&eth_config, &handle));

	setMacAddress(MacAddress({0x02, 0x00, 0x00, 0x12, 0x34, 0x56}));

	netif_glue = esp_eth_new_netif_glue(handle);
	CHECK_RET(esp_netif_attach(netif, netif_glue));
	CHECK_RET(esp_eth_start(handle));

	return true;
}

} // namespace Ethernet
