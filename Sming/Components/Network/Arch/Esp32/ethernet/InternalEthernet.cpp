/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Ethernet.cpp
 *
 ****/

#include <Platform/InternalEthernet.h>
// #include <freertos/event_groups.h>
#include <esp_eth.h>
#include <esp_event.h>
#include <driver/gpio.h>

bool InternalEthernet::begin(PhyFactory* phyFactory)
{
	if(phyFactory == nullptr) {
		debug_e("[ETH] InternalEthernet requires PHY");
		return false;
	}
	this->phyFactory.reset(phyFactory);

	esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
	esp_netif_t* eth_netif = esp_netif_new(&cfg);

	// Set default handlers to process TCP/IP stuffs
	ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
	enableEventCallback(true);
	enableGotIpCallback(true);

	eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();

	// CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
	// mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
	// mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
	mac = esp_eth_mac_new_esp32(&mac_config);
	phy = reinterpret_cast<esp_eth_phy_t*>(phyFactory->create());
	if(phy == nullptr) {
		debug_e("[ETH] Failed to construct PHY");
		return false;
	}

	// #if CONFIG_EXAMPLE_ETH_PHY_IP101
	// 	esp_eth_phy_t* phy = esp_eth_phy_new_ip101(&phy_config);
	// #elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
	// 	esp_eth_phy_t* phy = esp_eth_phy_new_rtl8201(&phy_config);
	// #elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
	// 	esp_eth_phy_t* phy = esp_eth_phy_new_lan8720(&phy_config);
	// #elif CONFIG_EXAMPLE_ETH_PHY_DP83848
	// 	esp_eth_phy_t* phy = esp_eth_phy_new_dp83848(&phy_config);
	// #elif CONFIG_EXAMPLE_ETH_PHY_KSZ8041
	// 	esp_eth_phy_t* phy = esp_eth_phy_new_ksz8041(&phy_config);
	// #endif

	esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
	ESP_ERROR_CHECK(esp_eth_driver_install(&config, &handle));

	/* attach Ethernet driver to TCP/IP stack */
	ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(handle)));
	/* start Ethernet driver state machine */
	ESP_ERROR_CHECK(esp_eth_start(handle));

	return true;
}

void InternalEthernet::end()
{
	if(handle == nullptr) {
		return;
	}

	ESP_ERROR_CHECK(esp_eth_stop(handle));
	// wait for connection stop
	// auto event_group = xEventGroupCreate();
	// auto bits = xEventGroupWaitBits(event_group, ETH_STOP_BIT, true, true, pdMS_TO_TICKS(ETH_STOP_TIMEOUT_MS));
	// vEventGroupDelete(event_group);
	// TEST_ASSERT((bits & ETH_STOP_BIT) == ETH_STOP_BIT);
	// driver should be uninstalled within 2 seconds
	ESP_ERROR_CHECK(esp_eth_driver_uninstall(handle));
	handle = nullptr;

	phyFactory->destroy(reinterpret_cast<PhyInstance*>(phy));

	ESP_ERROR_CHECK(mac->del(mac));
	mac = nullptr;

	enableEventCallback(false);
	enableGotIpCallback(false);
}

void InternalEthernet::enableEventCallback(bool enable)
{
	auto handler = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
		auto ethernet = static_cast<InternalEthernet*>(arg);
		ethernet->state = EthernetEvent(event_id);
		if(!ethernet->eventCallback) {
			return;
		}
		auto eth_handle = *static_cast<esp_eth_handle_t*>(event_data);
		MacAddress mac;
		esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, &mac[0]);
		ethernet->eventCallback(EthernetEvent(event_id), mac);
	};

	if(enable) {
		auto err = esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, handler, this);
		ESP_ERROR_CHECK(err);
	} else {
		esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, handler);
	}
}

void InternalEthernet::enableGotIpCallback(bool enable)
{
	auto handler = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
		auto ethernet = static_cast<InternalEthernet*>(arg);
		if(!ethernet->gotIpCallback) {
			return;
		}
		ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
		auto& ip_info = event->ip_info;
		ethernet->gotIpCallback(ip_info.ip.addr, ip_info.netmask.addr, ip_info.gw.addr);
	};

	if(enable) {
		auto err = esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, handler, this);
		ESP_ERROR_CHECK(err);
	} else {
		esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, handler);
	}
}
