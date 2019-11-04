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

#include "lwipcfg.h"
#include <../pcapif.h>
#include "npcap.h"
#include <pcap.h>

#include <lwip/sys.h>
#include <lwip/init.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <netif/ethernet.h>
#include <lwip/timeouts.h>

struct net_config {
	char ifname[128];
	unsigned ifindex;
	ip4_addr_t ipaddr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
};

static struct netif netif;

/*
 * Find an IP4 address in a list of addresses
 */
static bool find_ip4_addr(struct pcap_addr* pca, ip4_addr_t* addr, ip4_addr_t* mask)
{
#define IP4(saddr) ((ip4_addr_t*)&(((struct sockaddr_in*)saddr)->sin_addr))
	for(; pca != NULL; pca = pca->next) {
		if(pca->addr == NULL || pca->addr->sa_family != AF_INET) {
			continue;
		}
		addr->addr = ((struct sockaddr_in*)pca->addr)->sin_addr.s_addr;
		mask->addr = ((struct sockaddr_in*)pca->netmask)->sin_addr.s_addr;
		return true;
	}
	return false;
}

/*
 * param->ifname can be:
 *
 * 	A GUID                 doesn't need to be complete
 * 	An adapter number      0, 1, 2, etc.
 * 	NULL                   autodetect - if provided, will use param->ipaddr
 * 	?                      list adapters
 *
 */
static bool find_adapter(const struct lwip_param* param, struct net_config* netcfg)
{
	int ifindex = -1;

	if(param->ifname != NULL) {
		// Check for valid numeric argument, interpreted as the adapter number
		char* tail;
		ifindex = strtol(param->ifname, &tail, 0);
		if(*tail != '\0') {
			ifindex = -1; // Not a valid number
		}
	}

	pcap_if_t* alldevs;
	char errbuf[PCAP_ERRBUF_SIZE + 1];
	if(pcap_findalldevs(&alldevs, errbuf) < 0) {
		hostmsg("Error in pcap_findalldevs: %s", errbuf);
		return false;
	}

	if(param->ifname != NULL && param->ifname[0] == '?') {
		printf("Available adapters:\n");
	}

	bool res = false;

	pcap_if_t* d = alldevs;
	int idx = 0;
	for(; d != NULL; d = d->next, idx++) {
		if(param->ifname == NULL) {
			// @todo autodetect
			ip4_addr_t addr;
			ip4_addr_t mask;
			if(find_ip4_addr(d->addresses, &addr, &mask)) {
				if(ip4_addr_isany_val(addr)) {
					continue;
				}

				// If an IP address was requested, use that in our search for an appropriate adapter
				if(!ip4_addr_isany_val(netcfg->ipaddr) && !ip4_addr_netcmp(&netcfg->ipaddr, &addr, &mask)) {
					// Skip, address doesn't comply
					continue;
				}

				netcfg->ifindex = idx;
				strncpy(netcfg->ifname, d->description, sizeof(netcfg->ifname));
				netcfg->ifname[sizeof(netcfg->ifname) - 1] = '\0';
				netcfg->netmask.addr = mask.addr;
				res = true;
				break;
			}
		} else if(param->ifname[0] == '?') {
			const char* guid_ptr = strchr(d->name, '{');
			if(guid_ptr == NULL) {
				guid_ptr = d->name;
			}
			printf("- %d: %s", idx, guid_ptr);
			if(d->description != NULL) {
				printf(" - %s", d->description);
			}
			printf("\n");

			ip4_addr_t addr;
			ip4_addr_t mask;
			if(find_ip4_addr(d->addresses, &addr, &mask)) {
				char s[128];
				strcpy(s, ip4addr_ntoa(&addr));
				strcat(s, " / ");
				strcat(s, ip4addr_ntoa(&mask));
				printf("	%s\n", s);
			}
		} else if(idx == ifindex || (ifindex < 0 && strstr(d->name, param->ifname) != NULL)) {
			netcfg->ifindex = idx;
			strncpy(netcfg->ifname, d->description, sizeof(netcfg->ifname));
			netcfg->ifname[sizeof(netcfg->ifname) - 1] = '\0';
			if(ip4_addr_isany_val(netcfg->netmask)) {
				ip4_addr_t addr;
				find_ip4_addr(d->addresses, &addr, &netcfg->netmask);
			}
			res = true;
			break;
		}
	}

	pcap_freealldevs(alldevs);

	if(param->ifname != NULL && param->ifname[0] == '?') {
		exit(1);
	}

	return res;
}

bool host_lwip_init(const struct lwip_param* param)
{
	hostmsg("%s", "Initialising LWIP");

	if(!npcap_init()) {
		return false;
	}

	struct net_config netcfg = {0};

	if(param->ipaddr != NULL && ip4addr_aton(param->ipaddr, &netcfg.ipaddr) != 1) {
		hostmsg("Failed to parse IP address '%s'", param->ipaddr);
		return false;
	}

	if(param->netmask != NULL && ip4addr_aton(param->netmask, &netcfg.netmask) != 1) {
		hostmsg("Failed to parse Network Mask '%s'", param->netmask);
		return false;
	}

	if(param->gateway != NULL && ip4addr_aton(param->gateway, &netcfg.gw) != 1) {
		hostmsg("Failed to parse Gateway address '%s'", param->gateway);
		return false;
	}

	if(!find_adapter(param, &netcfg)) {
		return false;
	}

	char ip_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&netcfg.ipaddr, ip_str, sizeof(ip_str));
	char nm_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&netcfg.netmask, nm_str, sizeof(nm_str));
	char gw_str[IP4ADDR_STRLEN_MAX];
	ip4addr_ntoa_r(&netcfg.gw, gw_str, sizeof(gw_str));
	hostmsg("gateway = %s, netmask = %s; using ip = %s", gw_str, nm_str, ip_str);

	// Even though we're running as NO_SYS, stuff like crypt needs initialising
	sys_init();
	lwip_init();

	void* state = (void*)(netcfg.ifindex + 1); // See pcapif_low_level_init()
	netif_add(&netif, &netcfg.ipaddr, &netcfg.netmask, &netcfg.gw, state, pcapif_init, ethernet_input);
	netif_set_default(&netif);

	hostmsg("MAC: %02x:%02x:%02x:%02x:%02x:%02x", netif.hwaddr[0], netif.hwaddr[1], netif.hwaddr[2], netif.hwaddr[3],
			netif.hwaddr[4], netif.hwaddr[5]);

	return true;
}

void host_lwip_service(void)
{
	/* check for packets and link status*/
	pcapif_poll(&netif);
	sys_check_timeouts();
}

void host_lwip_shutdown(void)
{
	/* release the pcap library... */
	pcapif_shutdown(&netif);
}
