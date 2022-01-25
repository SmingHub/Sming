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
#include "threads.h"
#include "except.h"
#include "options.h"
#include <host_rboot.h>
#include <spi_flash/flashmem.h>
#include <driver/uart_server.h>
#include <BitManipulations.h>
#include <driver/os_timer.h>
#include <driver/hw_timer.h>
#include <esp_tasks.h>
#include <stdlib.h>
#include "include/hostlib/init.h"
#include "include/hostlib/emu.h"
#include "include/hostlib/hostlib.h"
#include "include/hostlib/CommandLine.h"
#include <Storage.h>
#include <Platform/System.h>

#ifndef DISABLE_NETWORK
#include <host_lwip.h>
#endif

CSemaphore host_main_loop_semaphore;

namespace
{
static int exitCode;
static bool done;

void cleanup()
{
	hw_timer_cleanup();
	host_flashmem_cleanup();
	UartServer::shutdown();
	sockets_finalise();
#ifndef DISABLE_NETWORK
	host_lwip_shutdown();
#endif
	host_debug_i("Goodbye!");
}

} // namespace

void host_exit(int code)
{
	static unsigned exit_count;

	host_debug_i("returning %d", code);
	exitCode = code;
	host_main_loop_semaphore.post();
	done = true;

	if(exit_count++) {
		host_debug_w("Forcing exit");
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
		(void)getchar();
	} else if(secs > 0) {
		hostmsg("Waiting for %u seconds...", secs);
		msleep(secs * 1000);
	}
}

/*
 * When there is no work being done we should wait efficiently.
 * Tasks and timers can be set from an interrupt (i.e. hardware thread),
 * so they can kick a semaphore to wake us up.
 */
int host_main_loop()
{
	system_soft_wdt_feed();
	host_service_tasks();
	return host_service_timers();
}

int main(int argc, char* argv[])
{
	trap_exceptions();

	struct Config {
		int pause{-1};
		int exitpause{-1};
		int loopcount;
		uint8_t cpulimit;
		bool initonly;
		bool enable_network{true};
		UartServer::Config uart;
		FlashmemConfig flash;
#ifndef DISABLE_NETWORK
		struct lwip_param lwip;
#endif
	};
	static Config config{};

	int uart_num{-1};
	option_tag_t opt;
	const char* arg;
	while((opt = get_option(argc, argv, arg)) != opt_none) {
		switch(opt) {
		case opt_help:
			print_help();
			return 0;

		case opt_uart:
			uart_num = atoi(arg);
			if(uart_num < 0 || uart_num >= UART_COUNT) {
				host_printf("UART %d number invalid\r\n", uart_num);
				return 0;
			}
			bitSet(config.uart.enableMask, uart_num);
			break;

		case opt_device:
		case opt_baud:
			if(uart_num < 0) {
				host_printf("--uart option missing\r\n");
				return 0;
			}
			if(opt == opt_device) {
				config.uart.deviceNames[uart_num] = arg;
			} else if(opt == opt_baud) {
				config.uart.baud[uart_num] = atoi(arg);
			}
			break;

		case opt_portbase:
			config.uart.portBase = atoi(arg);
			break;

#ifdef DISABLE_NETWORK
		case opt_ifname:
		case opt_ipaddr:
		case opt_gateway:
		case opt_netmask:
			break;
#else
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
#endif

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

		case opt_loopcount:
			config.loopcount = atoi(arg);
			break;

		case opt_nonet:
			config.enable_network = false;
			break;

		case opt_debug:
			host_debug_level = atoi(arg);
			break;

		case opt_cpulimit:
			config.cpulimit = atoi(arg);
			break;

		case opt_none:
			break;
		}
	}

	host_debug_i("\nWelcome to the Sming Host emulator\n\n");

	auto i = get_first_non_option();
	commandLine.parse(argc - i, &argv[i]);

	if(!host_flashmem_init(config.flash)) {
		return 1;
	}

	host_init_bootloader();

	atexit(cleanup);

	if(config.initonly) {
		host_debug_i("Initialise-only requested");
	} else {
		Storage::initialize();

		CThread::startup(config.cpulimit);

		hw_timer_init();

		host_init_tasks();

		sockets_initialise();
		UartServer::startup(config.uart);

#ifndef DISABLE_NETWORK
		if(config.enable_network) {
			host_lwip_init(config.lwip);
		} else {
			host_debug_i("Network initialisation skipped as requested");
		}
#endif

		host_debug_i("If required, you may start terminal application(s) now");
		pause(config.pause);

		host_debug_i(">> Starting Sming <<\n");

		System.initialize();

		host_init();

		while(!done) {
			int due = host_main_loop();
			if(config.loopcount != 0) {
				--config.loopcount;
				if(config.loopcount == 0) {
					host_debug_i("Reached requested loop count limit: exiting");
					break;
				}
			}
			constexpr int SCHED_WAIT{2};
			if(due > SCHED_WAIT) {
				host_main_loop_semaphore.timedwait((due - SCHED_WAIT) * 1000);
			}
		}

		host_debug_i(">> Normal Exit <<\n");
	}

	pause(config.exitpause);

	// Avoid issues with debug statements whilst running exit handlers
	m_setPuts(nullptr);

	return exitCode;
}
