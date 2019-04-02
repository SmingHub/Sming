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

#ifndef _GDB_HOOKS_H_
#define _GDB_HOOKS_H_

#include <user_config.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * 	- Debug exceptions will be silently ignored.
 * 	- System exceptions will cause a watchdog reset, as for `GDBSTUB_BREAK_ON_EXCEPTION = 0`.
 *	- All incoming serial data is passed through to UART2 without inspection,
 *		as for `GDBSTUB_CTRLC_BREAK = 0` and `GDBSTUB_FREE_ATTACH = 0`.
 */
void gdb_enable(bool state);

/**
 * @brief Break into GDB, if present
 * @note When present, gdbstub triggers entry into the debugger, which looks like a breakpoint hit.
 */
void gdb_do_break(void);

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
 * @brief Called on unexpected system reset
 */
void debug_crash_callback(const struct rst_info* rst_info, uint32_t stack, uint32_t stack_end);

/**
 * @brief Send a stack dump to debug output
 * @param start
 * @param end
 */
void debug_print_stack(uint32_t start, uint32_t end);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _GDB_HOOKS_H_ */
