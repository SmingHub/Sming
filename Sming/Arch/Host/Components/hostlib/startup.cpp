/**
 * startup.cpp - Sming Host Emulator startup code
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

// Prevent inclusion of networking, conflicts with sockets
#define __USER_CONFIG_H__

#include "sockets.h"
#include "except.h"
#include "options.h"
#include <spi_flash/flashmem.h>
#include <driver/uart_server.h>
#include <BitManipulations.h>
#include <esp_timer_legacy.h>
#include <esp_tasks.h>
#include "host_lwip.h"
#include <stdlib.h>
#include <esp_wifi.h>

#include <Platform/System.h>

static int exitCode = 0;
static bool done = false;

extern void init();

static void cleanup()
{
	host_flashmem_cleanup();
	CUartServer::shutdown();
	sockets_finalise();
	host_lwip_shutdown();
	hostmsg("Goodbye!");
}

void host_exit(int code)
{
	static unsigned exit_count;

	hostmsg("returning %d", code);
	exitCode = code;
	done = true;

	if(exit_count++) {
		hostmsg("Forcing exit");
		exit(exitCode);
	}
}

/*
 * Size may be specified as decimal, hex or with 'K' or 'M' suffix
 */
static size_t parse_flash_size(const char* str)
{
	if(str == nullptr) {
		return 0;
	}
	char* tail;
	long res = strtol(str, &tail, 0);
	if(res < 0) {
		return 0;
	}
	switch(*tail) {
	case 'k':
	case 'K':
		return size_t(res) * 1024U;
	case 'm':
	case 'M':
		return size_t(res) * 1024U * 1024U;
	default:
		return size_t(res);
	}
}

static void pause(int secs)
{
	if(secs == 0) {
		hostmsg("Hit ENTER to continue.");
		getchar();
	} else if(secs > 0) {
		hostmsg("Waiting for %u seconds...", secs);
		msleep(secs * 1000);
	}
}

int main(int argc, char* argv[])
{
	trap_exceptions();

	host_printf("\nWelcome to the Sming Host emulator\n\n");

	static struct {
		int pause;
		int exitpause;
		bool initonly;
		UartServerConfig uart;
		FlashmemConfig flash;
		struct lwip_param lwip;
	} config = {
		.pause = -1,
		.exitpause = -1,
		.initonly = false,
		.uart =
			{
				.enableMask = 0,
				.portBase = 0,
			},
		.flash =
			{
				.filename = nullptr,
				.createSize = 0,

			},
		.lwip =
			{
				.ifname = nullptr,
				.ipaddr = nullptr,
			},
	};

	option_tag_t opt;
	const char* arg;
	while((opt = get_option(argc, argv, arg)) != opt_none) {
		switch(opt) {
		case opt_help:
			print_help();
			return 0;

		case opt_uart:
			bitSet(config.uart.enableMask, atoi(arg));
			break;

		case opt_portbase:
			config.uart.portBase = atoi(arg);
			break;

		case opt_ifname:
			config.lwip.ifname = arg;
			break;

		case opt_ipaddr:
			config.lwip.ipaddr = arg;
			break;

		case opt_gateway:
			config.lwip.gateway = arg;
			break;

		case opt_netmask:
			config.lwip.netmask = arg;
			break;

		case opt_pause:
			config.pause = arg ? atoi(arg) : 0;
			break;

		case opt_exitpause:
			config.exitpause = arg ? atoi(arg) : 0;
			break;

		case opt_flashfile:
			config.flash.filename = arg;
			break;

		case opt_flashsize:
			config.flash.createSize = parse_flash_size(arg);
			break;

		case opt_initonly:
			config.initonly = true;
			break;

		default:;
		}
	}

	if(!host_flashmem_init(config.flash)) {
		return 1;
	}

	atexit(cleanup);

	if(config.initonly) {
		hostmsg("Initialise-only requested");
	} else {
		host_init_tasks();

		sockets_initialise();
		CUartServer::startup(config.uart);

		bool lwip_initialised = host_lwip_init(&config.lwip);
		if(lwip_initialised) {
			host_wifi_lwip_init_complete();
		}

		hostmsg("If required, you may start terminal application(s) now");
		pause(config.pause);

		hostmsg(">> Starting Sming <<\n");

		System.initialize();

		init();

		while(!done) {
			host_service_tasks();
			host_service_timers();
			if(lwip_initialised) {
				host_lwip_service();
			}
			system_soft_wdt_feed();
		}

		hostmsg(">> Normal Exit <<\n");
	}

	pause(config.exitpause);

	return exitCode;
}
