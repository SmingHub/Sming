/**
 * esp_wifi.c
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/esp_wifi.h"
#include <lwip/netif.h>
#include <lwip/dhcp.h>

static wifi_event_handler_cb_t event_handler;

void wifi_set_event_handler_cb(wifi_event_handler_cb_t cb)
{
	event_handler = cb;
}

static void wifi_status_callback(struct netif* nif)
{
	if(event_handler == NULL || nif == NULL) {
		return;
	}

	static uint32_t prev_flags;

	uint32_t changed_flags = prev_flags ^ nif->flags;
	prev_flags = nif->flags;

	if(changed_flags & NETIF_FLAG_UP) {
		System_Event_t evt = {.event =
								  (nif->flags & NETIF_FLAG_UP) ? EVENT_STAMODE_CONNECTED : EVENT_STAMODE_DISCONNECTED};
		const char SSID[] = "Host WiFi";
		memcpy(evt.event_info.connected.ssid, SSID, sizeof(SSID));
		evt.event_info.connected.ssid_len = sizeof(SSID) - 1;
		memcpy(evt.event_info.connected.bssid, nif->hwaddr, 6);
		evt.event_info.connected.channel = 1;
		event_handler(&evt);
	}

	if(!ip4_addr_isany_val(nif->ip_addr)) {
		System_Event_t evt = {.event = EVENT_STAMODE_GOT_IP,
							  .event_info = {.got_ip = {
												 .ip.addr = nif->ip_addr.addr,
												 .mask.addr = nif->netmask.addr,
												 .gw.addr = nif->gw.addr,
											 }}};
		event_handler(&evt);
	}
}

static void status_callback(struct netif* netif)
{
	host_queue_callback((host_task_callback_t)wifi_status_callback, (uint32_t)netif);
}

// Called directly from startup code
void host_wifi_lwip_init_complete(void)
{
	struct netif* nif = netif_default;

	if(nif == NULL) {
		return;
	}

	netif_set_status_callback(nif, status_callback);

	netif_set_up(nif);
#if LWIP_IPV6
	netif_create_ip6_linklocal_address(&nif, 1);
#endif

	if(ip4_addr_isany_val(nif->ip_addr)) {
		dhcp_start(nif);
	}
}

bool wifi_get_ip_info(uint8 if_index, struct ip_info* info)
{
	struct netif* nif = netif_default;

	if(if_index != STATION_IF || nif == NULL) {
		return false;
	}

	info->ip.addr = nif->ip_addr.addr;
	info->gw.addr = nif->gw.addr;
	info->netmask.addr = nif->netmask.addr;
	return true;
}

bool wifi_get_macaddr(uint8 if_index, uint8* macaddr)
{
	struct netif* nif = netif_default;

	if(if_index != STATION_IF || nif == NULL) {
		return false;
	}

	memcpy(macaddr, nif->hwaddr, 6);
	return true;
}
