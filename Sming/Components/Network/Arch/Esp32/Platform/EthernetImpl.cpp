/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Ethernet.cpp
 *
 ****/

#include <Platform/Ethernet.h>
#include <esp_eth.h>
#include <esp_event.h>
#include <driver/gpio.h>

bool Ethernet::begin()
{
	auto err = esp_event_handler_register(
		ETH_EVENT, ESP_EVENT_ANY_ID,
		[](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
			auto ethernet = static_cast<Ethernet*>(arg);
			if(!ethernet->eventCallback) {
				return;
			}
			auto eth_handle = *static_cast<esp_eth_handle_t*>(event_data);
			MacAddress mac;
			esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, &mac[0]);
			ethernet->eventCallback(EthernetEvent(event_id), mac);
		},
		this);
	ESP_ERROR_CHECK(err);

	err = esp_event_handler_register(
		IP_EVENT, IP_EVENT_ETH_GOT_IP,
		[](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
			auto ethernet = static_cast<Ethernet*>(arg);
			if(!ethernet->gotIpCallback) {
				return;
			}
			ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
			auto& ip_info = event->ip_info;
			ethernet->gotIpCallback(ip_info.ip.addr, ip_info.netmask.addr, ip_info.gw.addr);
		},
		this);
	ESP_ERROR_CHECK(err);

	return true;
}
