/**
 * host_lwip.h - Sming Host LWIP network support
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

#pragma once

#include "include/host_lwip.h"
#include <hostlib/hostmsg.h>
#include <lwip/init.h>
#include <lwip/ip_addr.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lwip_net_config {
	char ifname[128];
	unsigned ifindex;
	ip4_addr_t ipaddr;
	ip4_addr_t netmask;
	ip4_addr_t gw;
};

struct netif* lwip_arch_init(struct lwip_net_config& config);
void lwip_arch_shutdown();

/*
 * Poll the LWIP stack.
 * Return true if data was processed, false otherwise.
 */
bool lwip_arch_service();

#ifdef __cplusplus
}
#endif
