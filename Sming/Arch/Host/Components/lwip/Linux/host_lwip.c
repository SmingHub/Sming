/**
 * host_lwip.cpp
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

#include "../host_lwip.h"
#include "../../hostlib/hostmsg.h"

#include <lwip/init.h>
#include <lwip/ip_addr.h>
#include <lwip/timeouts.h>

#include <string.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include <netif/etharp.h>
#include <netif/tapif.h>

struct net_config {
	char ifname[32];
	ip4_addr_t ipaddr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
};

static struct netif netif;

static void getMacAddress(const char* ifname, uint8_t hwaddr[6])
{
	struct ifreq ifr = {0};
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, ifname);

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	close(fd);

	memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);
}

/**
 * @brief Fetch address and network mask for an interface
 * @param ifname NULL to get first compatible interface
 */
static bool getifaddr(const char* ifname, struct net_config* netcfg)
{
	struct ifaddrs* list;
	if(getifaddrs(&list) < 0) {
		hostmsg("getifaddrs: %s", strerror(errno));
		return false;
	}

	bool res = false;

	for(struct ifaddrs* ifa = list; ifa != NULL; ifa = ifa->ifa_next) {
		if(ifa->ifa_addr == NULL) {
			continue;
		}
		if(ifa->ifa_addr->sa_family != AF_INET) {
			continue;
		}

		if(ifname == NULL) {
			if(memcmp(ifa->ifa_name, "tap", 3) != 0) {
				continue;
			}
		} else if(strcmp(ifa->ifa_name, ifname) != 0) {
			continue;
		}

		strcpy(netcfg->ifname, ifa->ifa_name);
		netcfg->gw.addr = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr;
		netcfg->netmask.addr = ((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr.s_addr;

		res = true;
		break;
	}

	freeifaddrs(list);
	return res;
}

bool host_lwip_init(const struct lwip_param* param)
{
	hostmsg("%s", "Initialising LWIP");

	struct net_config netcfg = {0};

	if(!getifaddr(param->ifname, &netcfg)) {
		if(param->ifname == NULL) {
			hostmsg("%s", "No compatible interface found");
		} else {
			hostmsg("Interface '%s' not found", param->ifname);
		}
		return false;
	}

	if(param->gateway != NULL && ip4addr_aton(param->gateway, &netcfg.gw) != 1) {
		hostmsg("Failed to parse provided Gateway address '%s'", param->gateway);
		return false;
	}

	if(param->netmask != NULL && ip4addr_aton(param->netmask, &netcfg.netmask) != 1) {
		hostmsg("Failed to parse provided Network Mask '%s'", param->netmask);
		return false;
	}

	if(param->ipaddr == NULL) {
		// Choose a default IP address
		IP4_ADDR(&netcfg.ipaddr, ip4_addr1(&netcfg.gw), ip4_addr2(&netcfg.gw), ip4_addr3(&netcfg.gw), 10);
	} else if(ip4addr_aton(param->ipaddr, &netcfg.ipaddr) != 1) {
		hostmsg("Failed to parse provided IP address '%s'", param->ipaddr);
		return false;
	}

	char ip_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&netcfg.ipaddr, ip_str, sizeof(ip_str));
	char nm_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&netcfg.netmask, nm_str, sizeof(nm_str));
	char gw_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&netcfg.gw, gw_str, sizeof(gw_str));
	hostmsg("Using interface '%s', gateway = %s, netmask = %s; using ip = %s", netcfg.ifname, gw_str, nm_str, ip_str);

	setenv("PRECONFIGURED_TAPIF", netcfg.ifname, true);

	lwip_init();

	netif_add(&netif, &netcfg.ipaddr, &netcfg.netmask, &netcfg.gw, NULL, tapif_init, ethernet_input);

	getMacAddress(netcfg.ifname, netif.hwaddr);
	hostmsg("MAC: %02x:%02x:%02x:%02x:%02x:%02x", netif.hwaddr[0], netif.hwaddr[1], netif.hwaddr[2], netif.hwaddr[3],
			netif.hwaddr[4], netif.hwaddr[5]);

	netif_set_default(&netif);

	return true;
}

void host_lwip_service(void)
{
	/* poll netif, pass packet to lwIP */
	tapif_select(&netif);
	sys_check_timeouts();
}

void host_lwip_shutdown(void)
{
}
