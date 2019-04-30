/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 * Standard definitions for gdbstub code modules, both C++ and assembler (.S only).
 *
 ****/

#pragma once

#include "xtensa/xtruntime-frames.h"
#include "gdbstub-cfg.h"

#define UINT32_T unsigned int

/*
 * Defines our debugging exception frame which is provided to GDB when paused.
 * Our exception handlers save register values here, restoring them on continuation.
 * Any register changes made by GDB are stored here.
 *
 * System exceptions are initially handled by the Xtensa HAL, which saves some (but not all)
 * registers into a `UserFrame` structure (see xtensa/xtruntime-frames.h).
 * Control passes to gdbstub_exception_handler, which copies those values plus some extra ones.
 *
 * Debug exceptions are handled entirely by gdbstub_debug_exception_entry, which saves
 * registers, calls gdbstub_handle_debug_exception, then restores them.
 *
 * Using the macros provided by xtensa allow changes to this structure to be reflected into
 * both C and assembler code.
 */
STRUCT_BEGIN
STRUCT_FIELD(UINT32_T, 4, GDBSR_, pc)
STRUCT_FIELD(UINT32_T, 4, GDBSR_, ps)
STRUCT_FIELD(UINT32_T, 4, GDBSR_, sar)
STRUCT_FIELD(UINT32_T, 4, GDBSR_, vpri)
STRUCT_AFIELD(UINT32_T, 4, GDBSR_, a, 16) // a0..a15
#define GDBSR_A(n) GDBSR_a + ((n)*4)
// These are added manually by the exception code; the HAL doesn't set these on an exception.
STRUCT_FIELD(UINT32_T, 4, GDBSR_, litbase)
STRUCT_FIELD(UINT32_T, 4, GDBSR_, sr176)
STRUCT_FIELD(UINT32_T, 4, GDBSR_, sr208)
STRUCT_FIELD(UINT32_T, 4, GDBSR_, cause)	///< Either EXCCAUSE or DBGCAUSE, depending on exception type
STRUCT_FIELD(UINT32_T, 4, GDBSR_, excvaddr) ///< Exception Virtual Address
STRUCT_END(GdbstubSavedRegisters)

#undef UINT32_T

// Don't include debug output for this module unless excplitly requested
#if GDBSTUB_ENABLE_DEBUG == 0
#undef DEBUG_BUILD
#define DEBUG_BUILD 0
#endif

#if(defined(ENABLE_GDB) && GDBSTUB_BREAK_ON_EXCEPTION) || ENABLE_EXCEPTION_DUMP
#define HOOK_SYSTEM_EXCEPTIONS
#endif

/*
 * Code memory allocation attributes
 */
#define ATTR_GDBINIT ICACHE_FLASH_ATTR

#if GDBSTUB_FORCE_IRAM
#define ATTR_GDBEXTERNFN IRAM_ATTR
#else
#define ATTR_GDBEXTERNFN ICACHE_FLASH_ATTR
#endif

// Section definitions for assembler
#define ASATTR_GDBINIT .section .irom0.text
#define ASATTR_GDBFN .section .iram.text
