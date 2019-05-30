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
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lwip_param {
	const char* ifname;  ///< Name of interface to use
	const char* ipaddr;  ///< Client IP address
	const char* gateway; ///< Network gateway address
	const char* netmask; ///< Network mask
};

bool host_lwip_init(const struct lwip_param* param);
void host_lwip_service(void);
void host_lwip_shutdown(void);

#ifdef __cplusplus
}
#endif
