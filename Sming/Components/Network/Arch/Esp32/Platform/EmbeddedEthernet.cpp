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
// #include <freertos/event_groups.h>
#include <esp_eth.h>
#include <esp_event.h>
#include <driver/gpio.h>

using namespace Ethernet;

bool EmbeddedEthernet::begin(const MacConfig& macConfig, PhyFactory* phyFactory, const PhyConfig& phyConfig)
{
#if !CONFIG_ETH_USE_ESP32_EMAC

	debug_e("[ETH] Internal MAC not available");
	return false;

#else

	if(phyFactory == nullptr) {
		debug_e("[ETH] EmbeddedEthernet requires PHY");
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
	if(macConfig.smiMdcPin != PIN_DEFAULT) {
		mac_config.smi_mdc_gpio_num = macConfig.smiMdcPin;
	}
	if(macConfig.smiMdioPin != PIN_DEFAULT) {
		mac_config.smi_mdio_gpio_num = macConfig.smiMdioPin;
	}
	mac = esp_eth_mac_new_esp32(&mac_config);
	if(mac == nullptr) {
		debug_e("[ETH] Failed to construct MAC");
		return false;
	}

	phy = reinterpret_cast<esp_eth_phy_t*>(phyFactory->create());
	if(phy == nullptr) {
		debug_e("[ETH] Failed to construct PHY");
		return false;
	}

	esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
	ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &handle));
	ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(handle)));
	ESP_ERROR_CHECK(esp_eth_start(handle));

	return true;
#endif
}

void EmbeddedEthernet::end()
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

void EmbeddedEthernet::enableEventCallback(bool enable)
{
	auto handler = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
		auto ethernet = static_cast<EmbeddedEthernet*>(arg);
		ethernet->state = Event(event_id);
		if(!ethernet->eventCallback) {
			return;
		}
		auto eth_handle = *static_cast<esp_eth_handle_t*>(event_data);
		MacAddress mac;
		esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, &mac[0]);
		ethernet->eventCallback(Event(event_id), mac);
	};

	if(enable) {
		auto err = esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, handler, this);
		ESP_ERROR_CHECK(err);
	} else {
		esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, handler);
	}
}

void EmbeddedEthernet::enableGotIpCallback(bool enable)
{
	auto handler = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
		auto ethernet = static_cast<EmbeddedEthernet*>(arg);
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

MacAddress EmbeddedEthernet::getMacAddress() const
{
	MacAddress addr;
	if(mac != nullptr) {
		mac->get_addr(mac, &addr[0]);
	}
	return addr;
}

bool EmbeddedEthernet::setMacAddress(const MacAddress& addr)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_addr(mac, &const_cast<MacAddress&>(addr)[0]) == ESP_OK;
}

bool EmbeddedEthernet::setSpeed(Speed speed)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_speed(mac, eth_speed_t(speed)) == ESP_OK;
}

bool EmbeddedEthernet::setFullDuplex(bool enable)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_duplex(mac, enable ? ETH_DUPLEX_FULL : ETH_DUPLEX_HALF) == ESP_OK;
}

bool EmbeddedEthernet::setLinkState(bool up)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_link(mac, up ? ETH_LINK_UP : ETH_LINK_DOWN) == ESP_OK;
}

bool EmbeddedEthernet::setPromiscuous(bool enable)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_promiscuous(mac, enable) == ESP_OK;
}
