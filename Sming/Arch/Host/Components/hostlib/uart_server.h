/**
 * uart_server.h - UART emulation using sockets
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <driver/uart.h>
#include "sockets.h"
#include "threads.h"

#define UART_SOCKET_PORT_BASE 10000 ///< Port for UART0

struct UartServerConfig {
	unsigned enableMask; ///< Bit mask for required servers
	unsigned portBase; ///< Base port address (optional)
};

/*
 * Each server allocates a thread to listen for incoming connections. Only one client
 * connection is permitted.
 *
 * For simplicity this implementation uses sockets for communication, so we can just use
 * telnet as the terminal application.
 *
 * A socket is opened for each uart on the system at startup. If no client is connected
 * any output is discarded.
 *
 */
class CUartServer : public CThread, public CServerSocket
{
public:
	/**
	 * @brief Start requested servers
	 * @param config
	 */
	static void startup(const UartServerConfig& config);

	static void shutdown();

	CUartServer(unsigned uart_nr) : uart_nr(uart_nr)
	{
	}

	~CUartServer();

protected:
	void onNotify(uart_t* uart, uart_notify_code_t code);
	int serviceRead();
	int serviceWrite();
	void* thread_routine() override;

private:
	static unsigned portBase;
	CSocket* socket = nullptr; ///< Connected client
	CSemaphore txsem;		   ///< Signals when there's data to be sent out
	unsigned uart_nr;		   ///< Which port we represent
	uart_t* uart = nullptr;	///< On set if port is open by application
};
