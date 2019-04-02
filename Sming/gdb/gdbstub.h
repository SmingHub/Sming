/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdbstub.h
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#ifndef _GDB_GDBSTUB_H_
#define _GDB_GDBSTUB_H_


#include "gdbstub-internal.h"
#include <gdb_hooks.h>

// GDB_xx macro versions required to ensure no flash access if requested
#if GDBSTUB_FORCE_IRAM
#define GDB_F(str) str
#define GDB_PROGMEM
#else
#define GDB_F(str) _F(str)
#define GDB_PROGMEM PROGMEM
#endif

#define gdbstub_do_break() asm("break 0,0")

// Additional debugging flags
enum GdbDebugFlag {
	DBGFLAG_DEBUG_EXCEPTION,  ///< For debug exceptions, cause is DBGCAUSE (see DebugCause bits)
	DBGFLAG_SYSTEM_EXCEPTION, ///< For system exceptions, cause is EXCCAUSE (see EXCCAUSE_* values)
	DBGFLAG_CTRL_BREAK,		  ///< Break caused by call to gdbstub_ctrl_break()
	DBGFLAG_PACKET_STARTED,   ///< Incoming packet detected by uart interrupt handler
};

enum SyscallState {
	syscall_ready,   ///< Ready for new syscall
	syscall_pending, ///< Syscall queued but not yet sent to GDB
	syscall_active,  ///< Syscall executing, awaiting response from GDB
};

// State information in shared global structure
struct gdb_state_t {
	bool attached;		  ///< true if GDB is attached to stub
	bool enabled;		  ///< Debugging may be disabled via gdb_enable()
	SyscallState syscall; ///< State of system call
	uint8_t flags;		  ///< Combination of GdbDebugFlag
	unsigned ack_count;   ///< For discarding of acknowledgement characters
};

extern volatile gdb_state_t gdb_state;
extern const uint8_t gdb_exception_signals[];

void gdbstub_init();
void gdbstub_handle_exception(UserFrame* frame);

#endif /* _GDB_GDBSTUB_H_ */
