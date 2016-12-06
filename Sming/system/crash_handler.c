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
#include "espinc/peri.h"

extern void __real_system_restart_local();

extern void ets_printf(const char*, ...);

void uart_write_char_d(char c);
static void uart0_write_char_d(char c);
static void uart1_write_char_d(char c);
static void print_stack(uint32_t start, uint32_t end);
//static void print_pcs(uint32_t start, uint32_t end);

extern void __custom_crash_callback( struct rst_info * rst_info, uint32_t stack, uint32_t stack_end ) {
}

extern void custom_crash_callback( struct rst_info * rst_info, uint32_t stack, uint32_t stack_end ) __attribute__ ((weak, alias("__custom_crash_callback")));

void __wrap_system_restart_local() {
    register uint32_t sp asm("a1");

    struct rst_info rst_info = {0};
    system_rtc_mem_read(0, &rst_info, sizeof(rst_info));

    if (rst_info.reason != REASON_SOFT_WDT_RST &&
        rst_info.reason != REASON_EXCEPTION_RST &&
        rst_info.reason != REASON_WDT_RST)
    {
        return;
    }

#ifndef SMING_RELEASE
    ets_install_putc1(&uart_write_char_d);

    if (rst_info.reason == REASON_EXCEPTION_RST) {
        ets_printf("\n\n***** Exception Reset (%d):\nepc1=0x%08x epc2=0x%08x epc3=0x%08x excvaddr=0x%08x depc=0x%08x\n",
            rst_info.exccause, rst_info.epc1, rst_info.epc2, rst_info.epc3, rst_info.excvaddr, rst_info.depc);
    }
    else if (rst_info.reason == REASON_SOFT_WDT_RST) {
        ets_printf("\n\n***** Software Watchdog Reset\n");
    }
#endif

    uint32_t stack_end;

    // amount of stack taken by interrupt or exception handler
    // and everything up to __wrap_system_restart_local
    // (determined empirically, might break)
    uint32_t offset = 0;
    if (rst_info.reason == REASON_SOFT_WDT_RST) {
        offset = 0x1b0;
    }
    else if (rst_info.reason == REASON_EXCEPTION_RST) {
        offset = 0x1a0;
    }
    else if (rst_info.reason == REASON_WDT_RST) {
        offset = 0x10;
    }

    stack_end = 0x3fffffb0;
    // it's actually 0x3ffffff0, but the stuff below ets_run
    // is likely not really relevant to the crash

#ifndef SMING_RELEASE
    print_stack(sp + offset, stack_end);
#endif

    custom_crash_callback( &rst_info, sp + offset, stack_end );

    os_delay_us(10000);
    __real_system_restart_local();
}


static void print_stack(uint32_t start, uint32_t end) {
	uint32_t pos = 0;

    ets_printf("\n================================================================\n");
    for (pos = start; pos < end; pos += 0x10) {
        uint32_t* values = (uint32_t*)(pos);

        // rough indicator: stack frames usually have SP saved as the second word
        bool looksLikeStackFrame = (values[2] == pos + 0x10);

        ets_printf("%08x:  %08x %08x %08x %08x %c\n",
            pos, values[0], values[1], values[2], values[3], (looksLikeStackFrame)?'<':' ');
    }
    ets_printf("================================================================\n");
}

void uart_write_char_d(char c) {
    uart0_write_char_d(c);
    uart1_write_char_d(c);
}

static void uart0_write_char_d(char c) {
    while (((USS(0) >> USTXC) & 0xff)) { }

    if (c == '\n') {
        USF(0) = '\r';
    }
    USF(0) = c;
}

static void uart1_write_char_d(char c) {
    while (((USS(1) >> USTXC) & 0xff) >= 0x7e) { }

    if (c == '\n') {
        USF(1) = '\r';
    }
    USF(1) = c;
}
