/*
 Adapted from Arduino for Sming.
 Original copyright note is kept below.

 Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <user_config.h>
#include "gdb/gdb_hooks.h"

extern void __real_system_restart_local();

extern void __custom_crash_callback( struct rst_info * rst_info, uint32_t stack, uint32_t stack_end ) {
}

extern void custom_crash_callback( struct rst_info * rst_info, uint32_t stack, uint32_t stack_end ) __attribute__ ((weak, alias("__custom_crash_callback")));

void __wrap_system_restart_local() {
    register uint32_t sp_reg __asm__("a1");
    uint32_t sp = sp_reg;

    struct rst_info rst_info = {0};
    system_rtc_mem_read(0, &rst_info, sizeof(rst_info));

    // amount of stack taken by interrupt or exception handler
    // and everything up to __wrap_system_restart_local
    // (determined empirically, might break)
    uint32_t offset = 0;
    switch(rst_info.reason) {
    case REASON_SOFT_WDT_RST:
        offset = 0x1b0;
        break;
    case REASON_EXCEPTION_RST:
        offset = 0x1a0;
        break;
    case REASON_WDT_RST:
        offset = 0x10;
        break;
    case REASON_DEFAULT_RST:
    case REASON_SOFT_RESTART:
    case REASON_DEEP_SLEEP_AWAKE:
    case REASON_EXT_SYS_RST:
    default:
    	return;
    }

    uint32_t stack_end = 0x3fffffb0;
    // it's actually 0x3ffffff0, but the stuff below ets_run
    // is likely not really relevant to the crash

	debug_crash_callback(&rst_info, sp + offset, stack_end);

    custom_crash_callback(&rst_info, sp + offset, stack_end);

    os_delay_us(10000);
    __real_system_restart_local();
}

