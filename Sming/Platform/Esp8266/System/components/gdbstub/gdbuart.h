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

/*
 * Receive a char from the uart. Uses polling and feeds the watchdog.
 */
int gdbReceiveChar();

/*
 * Send a block of data to the uart
 */
size_t gdbSendData(const void* data, size_t length);

/*
 * Send a char to the uart
 */
size_t gdbSendChar(char c);

/**
 * @brief Write a block of data to the GDB console
 * @param data
 * @param length
 * @retval size_t Always returns length
 * @note Data is encoded as a single 'O' packet. Should only be used when GDB is attached.
 */
size_t ATTR_GDBEXTERNFN gdbWriteConsole(const char* data, size_t length);

/**
 * @brief Send some user data from the user_uart TX buffer to the GDB serial port,
 * packetising it if necessary.
 * @retval size_t Number of characters still remaining in buffer
 * @note Data flows from user uart TX buffer to UART0 either during uart_write() call
 * (via notify callback) or via task callback queued from ISR. We don't do this inside
 * the ISR as all the code (including packetising) would need to be in IRAM.
 */
size_t gdbSendUserData();

/**
 * @brief Ensure all user data has been written to serial port
 */
void gdbFlushUserData();

#endif /* _GDB_GDBUART_H_ */
