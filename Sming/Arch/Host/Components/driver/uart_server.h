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
#include <hostlib/sockets.h>
#include <hostlib/threads.h>
#include <memory>

class SerialDevice;

/*
 * Manages a set of virtualised UARTs
 */
namespace UartServer
{
struct Config {
	unsigned enableMask;				 ///< Bit mask for required servers
	unsigned portBase;					 ///< Base port address (optional)
	const char* deviceNames[UART_COUNT]; ///< Map uart to host port
	unsigned baud[UART_COUNT];			 ///< Speed for physical serial device
};

class CUart;

/**
 * @brief Start requested servers
 * @param config
 */
void startup(const Config& config);

void shutdown();

/*
 * Base class for a UART
 *
 * Each server allocates a thread to handle one device.
 * If no client (i.e. application `uart`) is connected any output is discarded.
 *
 */
class CUart : public CThread
{
public:
	CUart(unsigned uart_nr);

	virtual void terminate();

	virtual void onNotify(smg_uart_t* uart, smg_uart_notify_code_t code);

protected:
	static const unsigned IDLE_SLEEP_MS{100};

	virtual int available() = 0;
	virtual int readBytes(void* buffer, size_t size) = 0;
	virtual int writeBytes(const void* data, size_t size) = 0;
	int serviceRead();
	int serviceWrite();

	CSemaphore txsem;			///< Signals when there's data to be sent out
	unsigned uart_nr;			///< Which port we represent
	smg_uart_t* uart = nullptr; ///< On set if port is open by application
};

/*
 * UART implementation using TCP socket for communication, so we can use telnet as a terminal application.
 */
class CUartPort : public CUart, public CServerSocket
{
public:
	CUartPort(unsigned uart_nr);

	void terminate() override;

protected:
	int available() override;
	int readBytes(void* buffer, size_t size) override;
	int writeBytes(const void* data, size_t size) override;
	void* thread_routine() override;

	CSocket* socket{nullptr}; ///< Connected client
};

/*
 * UART implementation using physical serial device on local machine
 */
class CUartDevice : public CUart
{
public:
	static constexpr unsigned DEFAULT_BAUD{115200};

	CUartDevice(unsigned uart_nr, const char* deviceName, unsigned baud);

	void terminate() override;

	void onNotify(smg_uart_t* uart, smg_uart_notify_code_t code) override;

protected:
	int available() override;
	int readBytes(void* buffer, size_t size) override;
	int writeBytes(const void* data, size_t size) override;
	void* thread_routine() override;

	std::unique_ptr<SerialDevice> device; ///< Physical device
	const char* deviceName{nullptr};	  ///< Physical device name
	unsigned baud_rate{0};				  ///< Command-line override for baud rate
	bool done{false};
};

}; // namespace UartServer
