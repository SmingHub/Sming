/**
 * lwip_arch.cpp
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

// Use implementations defined in <netinet/in.h>
#define lwip_htonl nthol
#define lwip_htons htons

#include "../lwip_arch.h"
#include <hostlib/hostmsg.h>
#include <lwip/timeouts.h>
#include <cstring>
#include <ifaddrs.h>
#include <cerrno>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netif/etharp.h>
extern "C" {
#include <netif/tapif.h>
}

#ifdef __APPLE__
#include <fcntl.h>
#include <net/if_dl.h>
// For some reason _ip_data from lwip/core/ip.c isn't found, so add it here.
#include <lwip/ip.h>
struct ip_globals ip_data;
#endif

namespace
{
struct netif net_if;

void getMacAddress(const char* ifname, uint8_t hwaddr[6])
{
	memset(hwaddr, 0, 6);

	if(ifname == nullptr) {
		return;
	}

	struct ifreq ifr {
	};
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

#ifdef __APPLE__
	struct ifaddrs* list;
	if(getifaddrs(&list)) {
		return;
	}
	for(auto ifa = list; ifa != nullptr; ifa = ifa->ifa_next) {
		if(ifa->ifa_addr == nullptr) {
			continue;
		}
		if(ifa->ifa_addr->sa_family != AF_LINK) {
			continue;
		}
		auto sdl = reinterpret_cast<const sockaddr_dl*>(ifa->ifa_addr);
		host_debug_w("sdl_alen = %u", sdl->sdl_alen);
		if(sdl->sdl_alen != 6) {
			continue;
		}
		memcpy(hwaddr, LLADDR(sdl), 6);
		break;
	}
	freeifaddrs(list);
#else
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	int res = ioctl(fd, SIOCGIFHWADDR, &ifr);
	close(fd);

	if(res == 0) {
		memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);
	}
#endif
}

#ifndef __APPLE__
/**
 * @brief Fetch address and network mask for an interface
 * @param ifname nullptr to get first compatible interface
 */
bool getifaddr(struct lwip_net_config& netcfg)
{
	struct ifaddrs* list;
	if(getifaddrs(&list) < 0) {
		host_debug_e("getifaddrs: %s", strerror(errno));
		return false;
	}

	bool res{false};

	for(auto ifa = list; ifa != nullptr; ifa = ifa->ifa_next) {
		if(ifa->ifa_addr == nullptr) {
			continue;
		}
		if(ifa->ifa_addr->sa_family != AF_INET) {
			continue;
		}

		if(*netcfg.ifname == '\0') {
			if(memcmp(ifa->ifa_name, "tap", 3) != 0) {
				continue;
			}
		} else if(strcmp(ifa->ifa_name, netcfg.ifname) != 0) {
			continue;
		}

		strncpy(netcfg.ifname, ifa->ifa_name, sizeof(netcfg.ifname) - 1);
		netcfg.gw.addr = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr;
		netcfg.netmask.addr = ((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr.s_addr;

		res = true;
		break;
	}

	freeifaddrs(list);
	return res;
}
#endif

} // namespace

struct netif* lwip_arch_init(struct lwip_net_config& netcfg)
{
#ifdef __APPLE__

	int fd = open("/dev/tap0", O_RDWR);
	if(fd < 0) {
		host_debug_e("/dev/tap0 not found");
		return nullptr;
	}
	close(fd);

	memcpy(netcfg.ifname, "tap0", 5);
	IP4_ADDR(&netcfg.gw, 192, 168, 13, 1);
	IP4_ADDR(&netcfg.netmask, 255, 255, 255, 0);

#else

	if(!getifaddr(netcfg)) {
		if(netcfg.ifname[0] == '\0') {
			host_debug_e("%s", "No compatible interface found");
		} else {
			host_debug_e("Interface '%s' not found", netcfg.ifname);
		}
		return nullptr;
	}

	setenv("PRECONFIGURED_TAPIF", netcfg.ifname, true);

#endif

	if(ip_addr_isany(&netcfg.ipaddr)) {
		// Choose a default IP address
		IP4_ADDR(&netcfg.ipaddr, (uint32_t)ip4_addr1(&netcfg.gw), (uint32_t)ip4_addr2(&netcfg.gw),
				 (uint32_t)ip4_addr3(&netcfg.gw), 10U);
	}

	lwip_init();
	netif_add(&net_if, &netcfg.ipaddr, &netcfg.netmask, &netcfg.gw, nullptr, tapif_init, ethernet_input);
	getMacAddress(netcfg.ifname, net_if.hwaddr);

	return &net_if;
}

bool lwip_arch_service()
{
	/* poll netif, pass packet to lwIP */
	int res = tapif_select(&net_if);
	netif_poll(&net_if);
	sys_check_timeouts();

	return res > 0;
}

void lwip_arch_shutdown()
{
}
