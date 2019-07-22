/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * GDB Stub and exception/crash dumping configuration
 *
 * Unless otherwise noted, settings only take effect when application is built with ENABLE_GDB=1
 *
 ****/

#pragma once

/**
 * When enabled, an exception or crash dumps a stack trace to debug output
 * Default is ON for debug builds, OFF for release builds
 *
 * Note: Not dependent upon ENABLE_GDB
 *
*/
#ifndef ENABLE_EXCEPTION_DUMP
#ifdef SMING_RELEASE
#define ENABLE_EXCEPTION_DUMP 0
#else
#define ENABLE_EXCEPTION_DUMP 1
#endif
#endif

/**
 * When enabled, an unexpected reset (i.e. system crash) dumps a stack trace to debug output
 * Default is ON for debug builds, OFF for release builds
 *
 * Note: Not dependent upon ENABLE_GDB
*/
#ifndef ENABLE_CRASH_DUMP
#ifdef SMING_RELEASE
#define ENABLE_CRASH_DUMP 0
#else
#define ENABLE_CRASH_DUMP 1
#endif
#endif

/**
 * When defined, GDB communications are echoed to UART1 for testing GDB stub operation.
 *
 * 0: No debug output
 * 1: Show decoded commands and responses
 * 2: Show packet content
 * 3: Show debug output for internal routines
 */
#ifndef GDBSTUB_ENABLE_DEBUG
#define GDBSTUB_ENABLE_DEBUG 0
#endif

/**
 * Espressif provide a patched version of GDB which emits only those registered present in the lx106.
 * Set to 0 if an unpatched version of GDB is used.
 */
#ifndef GDBSTUB_GDB_PATCHED
#define GDBSTUB_GDB_PATCHED 1
#endif

/**
 * Enable this to make the exception and debugging handlers switch to a private stack. This will use
 * up 1K of RAM, but may be useful if you're debugging stack or stack pointer corruption problems. It's
 * normally disabled because not many situations need it. If for some reason the GDB communication
 * stops when you run into an error in your code, try enabling this.
 */
#ifndef GDBSTUB_USE_OWN_STACK
#define GDBSTUB_USE_OWN_STACK 0
#define GDBSTUB_STACK_SIZE 256 // In dwords
#endif

/**
 * Enable this to cause the program to pause and wait for gdb to be connected when an exception is encountered.
 */
#ifndef GDBSTUB_BREAK_ON_EXCEPTION
#define GDBSTUB_BREAK_ON_EXCEPTION 1
#endif

/**
 * Enable this to cause the program to pause and wait for gdb to be connected when an unexpected system restart occurs.
 */
#ifndef GDBSTUB_BREAK_ON_RESTART
#define GDBSTUB_BREAK_ON_RESTART 1
#endif

/**
 * If this is defined, gdbstub will break the program when you press Ctrl-C in gdb.
 * It does this by monitoring for the 'x03' character in the serial receive routine. Any preceding
 * characters are passed through to the application via UART2.
 * If your application uses the serial port for terminal (text) communications you should be OK,
 * but binary transfers are likely to cause problems and this option should probably be disabled.
 * Instead, use GDBSTUB_BREAK_ON_INIT, or call gdb_do_break() in your application.
 *
 * Specify:
 *  0 to disable Ctrl+C break checking completely
 * 	1 to allow Ctrl+C break only when debugger is attached
 * 	2 to allow Ctrl+C break at any time. Ensure you have
 * 			set remote interrupt-on-connect on
 * 		in GDB command file, so it will send a Ctrl+C sequence when attempting to connect
 */
#ifndef GDBSTUB_CTRLC_BREAK
#define GDBSTUB_CTRLC_BREAK 2
#endif

/**
 * The GDB stub has exclusive access to UART0, so applications cannot use it directly and attempts to
 * open it will fail.
 *
 * If this option is enabled, the default serial port will be changed to UART2 to allow debug output
 * (from m_printf, debug_*, os_printf, etc.) to show up in your GDB session.
 *
 * Outside of GDB terminal applications should work as normal, with the following caveats:
 *
 * 		If GDBSTUB_BREAK_ON_INIT is defined, then at startup your application will display `$T05#b9` and stop.
 * 		A similar thing will happen if GDBSTUB_CTRLC_BREAK=2 and you type Ctrl+C.
 * 		Continue by typing `$D#44` (without the quotes), or exit the terminal and start GDB.
 *
 * See GDB remote serial protocol for further details.
 *
 * Disabling this option releases some IRAM. You may be instead able to use UART1 for debug output,
 * adding `Serial.setPort(UART_ID_1);` in your application's `init()` function.
*/
#ifndef GDBSTUB_ENABLE_UART2
#define GDBSTUB_ENABLE_UART2 1
#endif

/**
 * Enable gdb_syscall_* functions for use by application.
 * If undefined, calls will do nothing and return -1.
 */
#ifndef GDBSTUB_ENABLE_SYSCALL
#define GDBSTUB_ENABLE_SYSCALL 0
#endif

/**
 * Enable Host I/O capability, where files may be accessed via GDB command prompt using
 * `remote get`, `remote put` and `remote delete` commands.
 */
#ifndef GDBSTUB_ENABLE_HOSTIO
#define GDBSTUB_ENABLE_HOSTIO 1
#endif

/**
 * Enable this if you want the GDB stub to wait for you to attach GDB before running.
 * It does this by breaking in the init routine; use the gdb 'c' command (continue) to start the program.
 */
#ifndef GDBSTUB_BREAK_ON_INIT
#define GDBSTUB_BREAK_ON_INIT 1
#endif

/**
 * Some commands are not required by GDB, so if neccessary can be disabled to save memory.
 */
// Read/write individual registers
#ifndef GDBSTUB_CMDENABLE_P
#define GDBSTUB_CMDENABLE_P 1
#endif
// Write binary-encoded data
#ifndef GDBSTUB_CMDENABLE_X
#define GDBSTUB_CMDENABLE_X 1
#endif

/**
 * Specify a timeout (in milliseconds) when stub is reading from serial port.
 * Set to 0 to wait indefinitely.
 */
#ifndef GDBSTUB_UART_READ_TIMEOUT
#define GDBSTUB_UART_READ_TIMEOUT 0
#endif

/**
 * Wherever possible gdbstub code is placed in flash memory.
 * This is fine for most cases, but if debugging whilst flash is disabled or busy (eg during SPI operations
 * or flash write/erase) then you will need to enable this option to move stub code into IRAM.
 */
#ifndef GDBSTUB_FORCE_IRAM
#define GDBSTUB_FORCE_IRAM 0
#endif
