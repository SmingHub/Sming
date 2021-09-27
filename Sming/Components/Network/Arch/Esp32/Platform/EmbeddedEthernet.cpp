/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EmbeddedEthernet.cpp
 *
 ****/

#include <Platform/EmbeddedEthernet.h>
#include <esp_eth.h>
#include <esp_netif.h>
#include <esp_event.h>

using namespace Ethernet;

bool EmbeddedEthernet::begin(const Config& config)
{
#if !CONFIG_ETH_USE_ESP32_EMAC

	debug_e("[ETH] Internal MAC not available");
	return false;

#else

	esp_netif_init();
	esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
	netif = esp_netif_new(&cfg);

	// Set default handlers to process TCP/IP stuffs
	ESP_ERROR_CHECK(esp_eth_set_default_handlers(netif));

	// And register our own event handlers
	enableEventCallback(true);
	enableGotIpCallback(true);

	eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
	if(config.smiMdcPin != PIN_DEFAULT) {
		mac_config.smi_mdc_gpio_num = config.smiMdcPin;
	}
	if(config.smiMdioPin != PIN_DEFAULT) {
		mac_config.smi_mdio_gpio_num = config.smiMdioPin;
	}
	mac = esp_eth_mac_new_esp32(&mac_config);
	if(mac == nullptr) {
		debug_e("[ETH] Failed to construct MAC");
		return false;
	}

	phy = reinterpret_cast<esp_eth_phy_t*>(phyFactory.create(config.phy));
	if(phy == nullptr) {
		debug_e("[ETH] Failed to construct PHY");
		return false;
	}

	esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
	ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &handle));
	netif_glue = esp_eth_new_netif_glue(handle);
	ESP_ERROR_CHECK(esp_netif_attach(netif, netif_glue));
	ESP_ERROR_CHECK(esp_eth_start(handle));

	return true;
#endif
}
