/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * libc.c - amendments to C runtime code
 *
 */

#include <debug_progmem.h>
#include <gdb/gdb_hooks.h>

#ifndef __NEWLIB__

int* __errno(void)
{
	static int errno_s = 0;
	return &errno_s;
}

#endif

void __assert_func(const char* file, int line, const char* func, const char* what)
{
	(void)file;
	(void)what;
	SYSTEM_ERROR("ASSERT: %s %d", func, line);
	gdb_do_break();
	while(1) {
		//
	}
}
