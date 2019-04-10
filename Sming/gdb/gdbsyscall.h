/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdbsyscall.h - Support for GDB file I/O
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#ifndef _GDB_GDBSYSCALL_H_
#define _GDB_GDBSYSCALL_H_

#include "gdbstub.h"

/**
 * @brief Called from gdbSendUserData() when user data has finished sending
 */
void gdbstub_syscall_execute();

/**
 * @brief Command handler for 'F'
 * @param data first character after 'F'
 * @retval bool true means Ctrl-C was pressed
 * @note A file (or console) I/O request ends with receipt of an 'F' command:
 *
 * 	Fretcode,errno,Ctrl-C flag;call-specific attachment
 */
bool gdb_syscall_complete(const char* data);

#endif /* _GDB_GDBSYSCALL_H_ */
