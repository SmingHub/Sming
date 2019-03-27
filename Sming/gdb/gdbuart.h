/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdbuart.h
 *
 ****/

#ifndef _GDB_GDBUART_H_
#define _GDB_GDBUART_H_

#include "gdbstub.h"

bool gdb_uart_init();

int gdbReceiveChar();
size_t gdbSendData(const void* data, size_t length);
size_t gdbSendChar(char c);

#endif /* _GDB_GDBUART_H_ */
