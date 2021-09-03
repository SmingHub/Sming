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

bool EmbeddedEthernet::begin(const Config& config)
{
#if !CONFIG_ETH_USE_ESP32_EMAC

	debug_e("[ETH] Internal MAC not available");
	return false;

#else

	esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
	netif = esp_netif_new(&cfg);

	// Set default handlers to process TCP/IP stuffs
	ESP_ERROR_CHECK(esp_eth_set_default_handlers(netif));

	// And register our own event handlers
	enableEventCallback(true);
	enableGotIpCallback(true);

	eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
	if(config.mac.smiMdcPin != PIN_DEFAULT) {
		mac_config.smi_mdc_gpio_num = config.mac.smiMdcPin;
	}
	if(config.mac.smiMdioPin != PIN_DEFAULT) {
		mac_config.smi_mdio_gpio_num = config.mac.smiMdioPin;
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

void EmbeddedEthernet::end()
{
	if(handle == nullptr) {
		return;
	}

	ESP_ERROR_CHECK(esp_eth_stop(handle));
	ESP_ERROR_CHECK(esp_eth_del_netif_glue(netif_glue));
	netif_glue = nullptr;
	ESP_ERROR_CHECK(esp_eth_clear_default_handlers(netif));

	ESP_ERROR_CHECK(esp_eth_driver_uninstall(handle));
	handle = nullptr;

	phyFactory.destroy(reinterpret_cast<PhyInstance*>(phy));
	phy = nullptr;

	ESP_ERROR_CHECK(mac->del(mac));
	mac = nullptr;

	esp_netif_destroy(netif);
	netif = nullptr;

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
		ethernet->eventCallback(Event(event_id));
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

void EmbeddedEthernet::setHostname(const String& hostname)
{
	ESP_ERROR_CHECK(esp_netif_set_hostname(netif, hostname.c_str()));
}

String EmbeddedEthernet::getHostname() const
{
	const char* hostName;
	ESP_ERROR_CHECK(esp_netif_get_hostname(netif, &hostName));
	return hostName;
}

IpAddress EmbeddedEthernet::getIP() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(netif, &info) == ESP_OK) {
		addr = info.ip.addr;
	}
	return addr;
}

extern "C" esp_err_t esp_netif_up(esp_netif_t* esp_netif);

bool EmbeddedEthernet::setIP(IpAddress address, IpAddress netmask, IpAddress gateway)
{
	if(!enableDHCP(false)) {
		return false;
	}
	esp_netif_ip_info_t ipinfo{address, netmask, gateway};
	if(esp_netif_set_ip_info(netif, &ipinfo) == ESP_OK) {
		debug_i("Ethernet IP successfully updated");
		esp_netif_up(netif);
	} else {
		debug_e("Ethernet IP can't be updated");
		enableDHCP(true);
	}
	return true;
}

bool EmbeddedEthernet::isEnabledDHCP() const
{
	esp_netif_dhcp_status_t status;
	if(esp_netif_dhcps_get_status(netif, &status) != ESP_OK) {
		return false;
	}

	return status == ESP_NETIF_DHCP_STARTED;
}

bool EmbeddedEthernet::enableDHCP(bool enable)
{
	if(enable) {
		return esp_netif_dhcpc_start(netif) == ESP_OK;
	} else {
		return esp_netif_dhcpc_stop(netif) == ESP_OK;
	}
}
