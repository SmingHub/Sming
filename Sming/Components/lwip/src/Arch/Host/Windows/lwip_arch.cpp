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

#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#include "../lwip_arch.h"
#include "lwipcfg.h"
#include <../pcapif.h>
#include "npcap.h"
#include <pcap.h>

#include <lwip/sys.h>
#include <lwip/netif.h>
#include <netif/ethernet.h>
#include <lwip/timeouts.h>

namespace
{
struct netif net_if;

/*
 * Find an IP4 address in a list of addresses
 */
bool find_ip4_addr(struct pcap_addr* pca, ip4_addr_t* addr, ip4_addr_t* mask)
{
#define IP4(saddr) ((ip4_addr_t*)&(((struct sockaddr_in*)saddr)->sin_addr))
	for(; pca != nullptr; pca = pca->next) {
		if(pca->addr == nullptr || pca->addr->sa_family != AF_INET) {
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
bool find_adapter(struct lwip_net_config& netcfg)
{
	pcap_if_t* alldevs;
	char errbuf[PCAP_ERRBUF_SIZE + 1];
	if(pcap_findalldevs(&alldevs, errbuf) < 0) {
		host_debug_e("Error in pcap_findalldevs: %s", errbuf);
		return false;
	}

	int ifindex = -1;
	switch(netcfg.ifname[0]) {
	case '?':
		printf("Available adapters:\n");
		break;
	case '\0':
		break;
	default:
		// Check for valid numeric argument, interpreted as the adapter number
		char* tail;
		ifindex = strtol(netcfg.ifname, &tail, 0);
		if(*tail != '\0') {
			ifindex = -1; // Not a valid number
		}
	}

	bool res{false};

	pcap_if_t* d = alldevs;
	int idx{0};
	for(; d != nullptr; d = d->next, idx++) {
		switch(netcfg.ifname[0]) {
		case '\0': {
			// @todo autodetect
			ip4_addr_t addr;
			ip4_addr_t mask;
			if(find_ip4_addr(d->addresses, &addr, &mask)) {
				if(ip4_addr_isany_val(addr)) {
					continue;
				}

				// If an IP address was requested, use that in our search for an appropriate adapter
				if(!ip4_addr_isany_val(netcfg.ipaddr) && !ip4_addr_netcmp(&netcfg.ipaddr, &addr, &mask)) {
					// Skip, address doesn't comply
					continue;
				}

				netcfg.ifindex = idx;
				strncpy(netcfg.ifname, d->description, sizeof(netcfg.ifname));
				netcfg.ifname[sizeof(netcfg.ifname) - 1] = '\0';
				netcfg.netmask.addr = mask.addr;
				res = true;
				break;
			}
			break;
		}

		case '?': {
			const char* guid_ptr = strchr(d->name, '{');
			if(guid_ptr == nullptr) {
				guid_ptr = d->name;
			}
			printf("- %d: %s", idx, guid_ptr);
			if(d->description != nullptr) {
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
			break;
		}

		default:
			if(idx != ifindex && strstr(d->name, netcfg.ifname) == nullptr) {
				// Interface match not found
				break;
			}

			netcfg.ifindex = idx;
			strncpy(netcfg.ifname, d->description, sizeof(netcfg.ifname) - 1);
			if(ip4_addr_isany_val(netcfg.netmask)) {
				ip4_addr_t addr;
				find_ip4_addr(d->addresses, &addr, &netcfg.netmask);
			}
			res = true;
			break;
		}
	}

	pcap_freealldevs(alldevs);

	if(netcfg.ifname[0] == '?') {
		// Terminate application once interfaces have been listed
		exit(1);
	}

	return res;
}

} // namespace

struct netif* lwip_arch_init(struct lwip_net_config& netcfg)
{
	if(!npcap_init()) {
		return nullptr;
	}

	if(!find_adapter(netcfg)) {
		return nullptr;
	}

	// Even though we're running as NO_SYS, stuff like crypt needs initialising
	sys_init();
	lwip_init();

	void* state = (void*)(netcfg.ifindex + 1); // See pcapif_low_level_init()
	netif_add(&net_if, &netcfg.ipaddr, &netcfg.netmask, &netcfg.gw, state, pcapif_init, ethernet_input);

	return &net_if;
}

bool lwip_arch_service()
{
	/* check for packets and link status*/
	pcapif_poll(&net_if);
	netif_poll(&net_if);
	sys_check_timeouts();

	return true;
}

void lwip_arch_shutdown()
{
	/* release the pcap library... */
	pcapif_shutdown(&net_if);
}
