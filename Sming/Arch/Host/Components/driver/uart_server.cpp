/**
 * uart_server.cpp
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

#include "uart_server.h"
#include <espinc/uart_register.h>
#include <SerialBuffer.h>
#include <BitManipulations.h>
#include <hostlib/keyb.h>

const unsigned IDLE_SLEEP_MS = 100;

unsigned CUartServer::portBase = 10000;

static CUartServer* uartServers[UART_COUNT];

class KeyboardThread : public CThread
{
public:
	KeyboardThread() : CThread("keyboard", 0)
	{
	}

	void terminate()
	{
		done = true;
		join();
	}

protected:
	void* thread_routine() override;

private:
	bool done = false;
};

void* KeyboardThread::thread_routine()
{
	// Small applications can complete before we even get here!
	msleep(50);
	if(done) {
		return nullptr;
	}

	keyb_raw();
	while(!done) {
		int c = getkey();
		if(c == KEY_NONE) {
			sched_yield();
			continue;
		}

		auto uart = uart_get_uart(UART0);
		assert(uart != nullptr);
		auto buf = uart->rx_buffer;
		assert(buf != nullptr);
		do {
			buf->writeChar(c);
		} while((c = getkey()) != KEY_NONE);

		uart->status |= UART_RXFIFO_TOUT_INT_ST;

		interrupt_begin();

		auto status = uart->status;
		uart->status = 0;
		if(status != 0 && uart->callback != nullptr) {
			uart->callback(uart, status);
		}

		interrupt_end();
	}
	keyb_restore();
	return nullptr;
}

static KeyboardThread* keyboardThread;

static void destroyKeyboardThread()
{
	if(keyboardThread == nullptr) {
		return;
	}

	keyboardThread->terminate();
	delete keyboardThread;
	keyboardThread = nullptr;
}

static void onUart0Notify(uart_t* uart, uart_notify_code_t code)
{
	switch(code) {
	case UART_NOTIFY_AFTER_WRITE: {
		size_t avail;
		void* data;
		while((avail = uart->tx_buffer->getReadData(data)) != 0) {
			host_nputs(static_cast<const char*>(data), avail);
			uart->tx_buffer->skipRead(avail);
		}
		break;
	}

	case UART_NOTIFY_AFTER_OPEN:
		if(uart_rx_enabled(uart)) {
			assert(keyboardThread == nullptr);
			keyboardThread = new KeyboardThread;
			keyboardThread->execute();
		}
		break;

	case UART_NOTIFY_BEFORE_CLOSE:
		destroyKeyboardThread();
		break;

	default:; // ignore
	}
}

void CUartServer::startup(const UartServerConfig& config)
{
	if(config.portBase != 0) {
		portBase = config.portBase;
	}

	auto notify = [](uart_t* uart, uart_notify_code_t code) {
		auto server = uartServers[uart->uart_nr];
		if(server) {
			server->onNotify(uart, code);
		} else if(code == UART_NOTIFY_AFTER_WRITE) {
			uart->tx_buffer->clear();
		}
	};

	for(unsigned i = 0; i < UART_COUNT; ++i) {
		uart_set_notify(i, notify);
		if(!bitRead(config.enableMask, i)) {
			continue;
		}
		auto& server = uartServers[i];
		server = new CUartServer(i);
		server->execute();
	}

	// If no ports have been enabled then redirect port 0 output to host console
	if(config.enableMask == 0) {
		// Redirect the main serial port to console output
		uart_set_notify(UART0, onUart0Notify);
	}
}

void CUartServer::shutdown()
{
	destroyKeyboardThread();

	for(unsigned i = 0; i < UART_COUNT; ++i) {
		auto& server = uartServers[i];
		if(server == nullptr) {
			continue;
		}

		server->terminate();
		delete server;
		server = nullptr;
	}
}

void CUartServer::terminate()
{
	close();
	join();
	hostmsg("UART%u server destroyed", uart_nr);
}

void CUartServer::onNotify(uart_t* uart, uart_notify_code_t code)
{
	switch(code) {
	case UART_NOTIFY_AFTER_OPEN:
		this->uart = uart;
		break;

	case UART_NOTIFY_BEFORE_CLOSE:
		this->uart = nullptr;
		break;

	case UART_NOTIFY_AFTER_WRITE: {
		if(socket == nullptr) {
			// Not connected, discard data
			uart->tx_buffer->clear();
		} else {
			// Kick the thread to send now
			txsem.post();
		}
		break;
	}

	case UART_NOTIFY_WAIT_TX:
		break;

	case UART_NOTIFY_BEFORE_READ:
		break;
	}
}

int CUartServer::serviceRead()
{
	if(!uart_rx_enabled(uart)) {
		return 0;
	}

	int avail = socket->available();
	if(avail <= 0) {
		return avail;
	}

	interrupt_begin();

	int space = uart->rx_buffer->getFreeSpace();
	int read = std::min(space, avail);
	if(read != 0) {
		char buffer[read];
		read = socket->recv(buffer, read);
		if(read > 0) {
			for(int i = 0; i < read; ++i) {
				uart->rx_buffer->writeChar(buffer[i]);
			}
			space -= read;
			if(space == 0) {
				uart->status |= UART_RXFIFO_FULL_INT_ST;
			} else {
				uart->status |= UART_RXFIFO_TOUT_INT_ST;
			}
		}
	}

	interrupt_end();

	return read;
}

int CUartServer::serviceWrite()
{
	if(!uart_tx_enabled(uart)) {
		return 0;
	}

	int result = 0;
	void* data;
	auto txbuf = uart->tx_buffer;
	size_t avail = txbuf->getReadData(data);
	if(avail == 0) {
		return 0;
	}

	interrupt_begin();

	do {
		int sent = socket->send(data, avail);
		if(sent < 0) {
			hostmsg("Uart send returned %d", sent);
			result = sent;
			break;
		}
		txbuf->skipRead(sent);
		result += sent;
	} while((avail = txbuf->getReadData(data)) != 0);

	if(txbuf->isEmpty()) {
		uart->status |= UART_TXFIFO_EMPTY_INT_ST;
	} else {
		txsem.post();
	}

	interrupt_end();

	return result;
}

void* CUartServer::thread_routine()
{
	auto port = portBase + uart_nr;
	CSockAddr addr(nullptr, port);
	if(!listen(addr, 1)) {
		hostmsg("Listen %s failed", addr.text().c_str());
		return nullptr;
	}

	hostmsg("UART%u server listening on port %u", uart_nr, port);

	while(active()) {
		socket = try_connect();
		if(socket == nullptr) {
			msleep(300);
			continue;
		}

		hostmsg("Uart #%u socket open", uart_nr);

		while(socket->active()) {
			if(txsem.timedwait(IDLE_SLEEP_MS)) {
				if(serviceWrite() < 0) {
					break;
				}
			}

			if(serviceRead() < 0) {
				break;
			}

			if(uart != nullptr) {
				interrupt_begin();

				auto status = uart->status;
				uart->status = 0;
				if(status != 0 && uart->callback != nullptr) {
					uart->callback(uart, status);
				}

				interrupt_end();
			}
		}

		socket->close();
		hostmsg("Uart #%u socket closed", uart_nr);
	}

	return nullptr;
}
