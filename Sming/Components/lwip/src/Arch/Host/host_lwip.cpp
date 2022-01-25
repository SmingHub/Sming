/**
 * common.cpp
 *
 * Copyright 2022 mikee47 <mike@sillyhouse.net>
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "lwip_arch.h"
#include "lwip/netif.h"
#include <SimpleTimer.h>

namespace
{
SimpleTimer lwipServiceTimer;
host_lwip_init_callback_t init_callback;

// Service stack more freqently when busy to ensure decent throughput
constexpr unsigned activeInterval{2};
constexpr unsigned inactiveInterval{100};

} // namespace

bool host_lwip_init(const struct lwip_param& param)
{
	host_debug_i("%s", "Initialising LWIP");

	lwip_net_config config{};
	if(param.ifname != nullptr) {
		strncpy(config.ifname, param.ifname, sizeof(config.ifname) - 1);
	}

	if(param.netmask != nullptr && ip4addr_aton(param.netmask, &config.netmask) != 1) {
		host_debug_e("Failed to parse Network Mask '%s'", param.netmask);
		return false;
	}

	if(param.gateway != nullptr && ip4addr_aton(param.gateway, &config.gw) != 1) {
		host_debug_e("Failed to parse Gateway address '%s'", param.gateway);
		return false;
	}

	if(param.ipaddr != nullptr && ip4addr_aton(param.ipaddr, &config.ipaddr) != 1) {
		host_debug_e("Failed to parse IP address '%s'", param.ipaddr);
		return false;
	}

	auto nif = lwip_arch_init(config);
	if(nif == nullptr) {
		return false;
	}
	netif_set_default(nif);

#if DEBUG_VERBOSE_LEVEL >= INFO
	char ip_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&config.ipaddr, ip_str, sizeof(ip_str));
	char nm_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&config.netmask, nm_str, sizeof(nm_str));
	char gw_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&config.gw, gw_str, sizeof(gw_str));
	host_debug_i("Using interface '%s', gateway %s, netmask %s, ip %s", config.ifname, gw_str, nm_str, ip_str);

	assert(nif != nullptr);
	host_debug_i("MAC: %02x:%02x:%02x:%02x:%02x:%02x", nif->hwaddr[0], nif->hwaddr[1], nif->hwaddr[2], nif->hwaddr[3],
				 nif->hwaddr[4], nif->hwaddr[5]);
#endif

	if(init_callback != nullptr) {
		init_callback();
	}

	lwipServiceTimer.initializeMs(activeInterval, []() {
		bool active = lwip_arch_service();
		lwipServiceTimer.setIntervalMs(active ? activeInterval : inactiveInterval);
		lwipServiceTimer.startOnce();
	});
	lwipServiceTimer.startOnce();

	return true;
}

void host_lwip_shutdown()
{
	lwipServiceTimer.stop();
	lwip_arch_shutdown();
}

void host_lwip_on_init_complete(host_lwip_init_callback_t callback)
{
	init_callback = callback;
}
