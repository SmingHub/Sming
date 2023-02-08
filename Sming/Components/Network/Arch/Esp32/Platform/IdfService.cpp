/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * IdfService.cpp
 *
 ****/

#include <Platform/IdfService.h>
#include <esp_eth.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <debug_progmem.h>

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 4, 0)
using esp_eth_netif_glue_handle_t = void*;
#endif

namespace Ethernet
{
void IdfService::end()
{
	if(handle == nullptr) {
		return;
	}

	ESP_ERROR_CHECK(esp_eth_stop(handle));
	ESP_ERROR_CHECK(esp_eth_del_netif_glue(static_cast<esp_eth_netif_glue_handle_t>(netif_glue)));
	netif_glue = nullptr;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 4, 0)
	ESP_ERROR_CHECK(esp_eth_clear_default_handlers(netif));
#endif

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

void IdfService::enableEventCallback(bool enable)
{
	auto handler = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
		auto service = static_cast<IdfService*>(arg);
		service->state = Event(event_id);
		if(!service->eventCallback) {
			return;
		}
		service->eventCallback(Event(event_id));
	};

	if(enable) {
		auto err = esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, handler, this);
		ESP_ERROR_CHECK(err);
	} else {
		esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, handler);
	}
}

void IdfService::enableGotIpCallback(bool enable)
{
	auto handler = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
		auto service = static_cast<IdfService*>(arg);
		if(!service->gotIpCallback) {
			return;
		}
		ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
		auto& ip_info = event->ip_info;
		service->gotIpCallback(ip_info.ip.addr, ip_info.netmask.addr, ip_info.gw.addr);
	};

	if(enable) {
		auto err = esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, handler, this);
		ESP_ERROR_CHECK(err);
	} else {
		esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, handler);
	}
}

MacAddress IdfService::getMacAddress() const
{
	MacAddress addr;
	if(mac != nullptr) {
		mac->get_addr(mac, &addr[0]);
	}
	return addr;
}

bool IdfService::setMacAddress(const MacAddress& addr)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_addr(mac, &const_cast<MacAddress&>(addr)[0]) == ESP_OK;
}

bool IdfService::setSpeed(Speed speed)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_speed(mac, eth_speed_t(speed)) == ESP_OK;
}

bool IdfService::setFullDuplex(bool enable)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_duplex(mac, enable ? ETH_DUPLEX_FULL : ETH_DUPLEX_HALF) == ESP_OK;
}

bool IdfService::setLinkState(bool up)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_link(mac, up ? ETH_LINK_UP : ETH_LINK_DOWN) == ESP_OK;
}

bool IdfService::setPromiscuous(bool enable)
{
	if(mac == nullptr) {
		return false;
	}
	return mac->set_promiscuous(mac, enable) == ESP_OK;
}

void IdfService::setHostname(const String& hostname)
{
	ESP_ERROR_CHECK(esp_netif_set_hostname(netif, hostname.c_str()));
}

String IdfService::getHostname() const
{
	const char* hostName;
	ESP_ERROR_CHECK(esp_netif_get_hostname(netif, &hostName));
	return hostName;
}

IpAddress IdfService::getIP() const
{
	IpAddress addr;
	esp_netif_ip_info_t info;
	if(esp_netif_get_ip_info(netif, &info) == ESP_OK) {
		addr = info.ip.addr;
	}
	return addr;
}

extern "C" esp_err_t esp_netif_up(esp_netif_t* esp_netif);

bool IdfService::setIP(IpAddress address, IpAddress netmask, IpAddress gateway)
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

bool IdfService::isEnabledDHCP() const
{
	esp_netif_dhcp_status_t status;
	if(esp_netif_dhcps_get_status(netif, &status) != ESP_OK) {
		return false;
	}

	return status == ESP_NETIF_DHCP_STARTED;
}

bool IdfService::enableDHCP(bool enable)
{
	if(enable) {
		return esp_netif_dhcpc_start(netif) == ESP_OK;
	} else {
		return esp_netif_dhcpc_stop(netif) == ESP_OK;
	}
}

} // namespace Ethernet
