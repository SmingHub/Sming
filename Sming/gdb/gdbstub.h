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

#include <user_config.h>

// GDB_xx macro versions required to ensure no flash access if requested
#if GDBSTUB_FORCE_IRAM
#define GDB_F(str) str
#define GDB_PROGMEM
#else
#define GDB_F(str) _F(str)
#define GDB_PROGMEM PROGMEM
#endif

#define gdbstub_do_break() asm("break 0,0")

extern bool gdb_attached;
extern bool gdb_enabled;
extern const uint8_t gdb_exception_signals[];

void gdbstub_init();
void gdbstub_handle_exception(UserFrame* frame);
void gdbstub_ctrl_break();

#endif /* _GDB_GDBSTUB_H_ */
