/*
 gdb_hooks.h - Hooks for GDB Stub library
 Copyright (c) 2018 Ivan Grokhotkov.  All right reserved.

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

#pragma once

#include <user_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup GDB GDB debugging support
 *  @{
*/

/**
 * @brief Initialise GDB stub, if present
 * @note Called by framework at startup, but does nothing if gdbstub is not linked.
 */
void gdb_init(void);

/**
 * @brief Dynamically enable/disable GDB stub
 * @param state true to enable, false to disable
 * @note has no effect if gdbstub is not present
 *
 * Calling with `enable = false` overrides configured behaviour as follows:
 *
 * - Debug exceptions will be silently ignored
 * - System exceptions will cause a watchdog reset, as for `GDBSTUB_BREAK_ON_EXCEPTION = 0`
 * - All incoming serial data is passed through to UART2 without inspection, as for `GDBSTUB_CTRLC_BREAK = 0`
 *
 */
void gdb_enable(bool state);

/**
 * @brief Break into GDB, if present
 */
#ifdef ENABLE_GDB
#define gdb_do_break() __asm__("break 0,0")
#else
#define gdb_do_break()                                                                                                 \
	do {                                                                                                               \
	} while(0)
#endif

typedef enum {
	eGDB_NotPresent,
	eGDB_Detached,
	eGDB_Attached,
} GdbState;

/**
 * @brief Check if GDB stub is present
 */
GdbState gdb_present(void);

/**
 * @brief Called from task queue when GDB attachment status changes
 * @note User can implement this function to respond to attachment changes
 */
void gdb_on_attach(bool attached);

/**
 * @brief Detach from GDB, if attached
 * @note We send GDB an 'exit process' message
 */
void gdb_detach();

/*
 * @brief Called by framekwork on unexpected system reset
 */
void debug_crash_callback(const struct rst_info* rst_info, uint32_t stack, uint32_t stack_end);

/**
 * @brief Send a stack dump to debug output
 * @param start Start address to output
 * @param end Output up to - but not including - this address
 */
void debug_print_stack(uint32_t start, uint32_t end);

#ifdef __cplusplus
} // extern "C"
#endif

/** @} */
