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
#include <driver/SerialBuffer.h>
#include <BitManipulations.h>
#include <hostlib/keyb.h>
#include <SerialLib.h>
#include <cassert>

namespace UartServer
{
namespace
{
unsigned portBase{10000};
std::unique_ptr<CUart> servers[UART_COUNT];
} // namespace

class KeyboardThread : public CThread
{
public:
	KeyboardThread() : CThread("keyboard", 1)
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

		auto uart = smg_uart_get_uart(UART0);
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

static void onUart0Notify(smg_uart_t* uart, smg_uart_notify_code_t code)
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
		if(smg_uart_rx_enabled(uart)) {
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

void startup(const Config& config)
{
	if(config.portBase != 0) {
		portBase = config.portBase;
	}

	auto notify = [](smg_uart_t* uart, smg_uart_notify_code_t code) {
		auto& server = servers[uart->uart_nr];
		if(server) {
			server->onNotify(uart, code);
		} else if(code == UART_NOTIFY_AFTER_WRITE) {
			uart->tx_buffer->clear();
		}
	};

	for(unsigned i = 0; i < UART_COUNT; ++i) {
		smg_uart_set_notify(i, notify);
		if(!bitRead(config.enableMask, i)) {
			continue;
		}
		auto& server = servers[i];
		if(config.deviceNames[i] == nullptr) {
			server.reset(new CUartPort(i));
		} else {
			server.reset(new CUartDevice(i, config.deviceNames[i], config.baud[i]));
		}
		server->execute();
	}

	// If no ports have been enabled then redirect port 0 output to host console
	if(config.enableMask == 0) {
		// Redirect the main serial port to console output
		smg_uart_set_notify(UART0, onUart0Notify);
	}
}

void shutdown()
{
	destroyKeyboardThread();

	for(unsigned i = 0; i < UART_COUNT; ++i) {
		auto& server = servers[i];
		if(!server) {
			continue;
		}

		server->terminate();
		server.reset();
	}
}

/* CUart */

CUart::CUart(unsigned uart_nr) : CThread("uart", 1), uart_nr(uart_nr)
{
}

void CUart::terminate()
{
	join();
	host_debug_i("UART%u server destroyed", uart_nr);
}

void CUart::onNotify(smg_uart_t* uart, smg_uart_notify_code_t code)
{
	switch(code) {
	case UART_NOTIFY_AFTER_OPEN:
		this->uart = uart;
		break;

	case UART_NOTIFY_BEFORE_CLOSE:
		this->uart = nullptr;
		break;

	case UART_NOTIFY_AFTER_WRITE: {
		if(this->uart != nullptr) {
			// Kick the thread to send now
			txsem.post();
		} else {
			// Not connected, discard data
			uart->tx_buffer->clear();
		}
		break;
	}

	case UART_NOTIFY_WAIT_TX:
	case UART_NOTIFY_BEFORE_READ:
		break;
	}
}

int CUart::serviceRead()
{
	if(!smg_uart_rx_enabled(uart)) {
		return 0;
	}

	int avail = available();
	if(avail <= 0) {
		return avail;
	}

	interrupt_begin();

	int space = uart->rx_buffer->getFreeSpace();
	if(space < avail) {
		uart->status |= UART_RXFIFO_OVF_INT_ST;
	}
	int read = std::min(space, avail);
	if(read != 0) {
		char buffer[read];
		read = readBytes(buffer, read);
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

int CUart::serviceWrite()
{
	if(!smg_uart_tx_enabled(uart)) {
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
		int sent = writeBytes(data, avail);
		if(sent < 0) {
			host_debug_w("Uart send returned %d", sent);
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

/* CUartPort */

CUartPort::CUartPort(unsigned uart_nr) : CUart(uart_nr)
{
}

void CUartPort::terminate()
{
	close();
	CUart::terminate();
}

int CUartPort::available()
{
	return socket ? socket->available() : 0;
}

int CUartPort::readBytes(void* buffer, size_t size)
{
	return socket ? socket->recv(buffer, size) : 0;
}

int CUartPort::writeBytes(const void* data, size_t size)
{
	return socket ? socket->send(data, size) : 0;
}

void* CUartPort::thread_routine()
{
	auto port = portBase + uart_nr;
	CSockAddr addr(nullptr, port);
	if(!listen(addr, 1)) {
		host_debug_e("Listen %s failed", addr.text().c_str());
		return nullptr;
	}

	host_debug_i("UART%u server listening on port %u", uart_nr, port);

	while(active()) {
		socket = try_connect();
		if(socket == nullptr) {
			msleep(300);
			continue;
		}

		host_debug_i("Uart #%u socket open", uart_nr);

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
		host_debug_i("Uart #%u socket closed", uart_nr);
	}

	return nullptr;
}

/* CUartDevice */

CUartDevice::CUartDevice(unsigned uart_nr, const char* deviceName, unsigned baud_rate)
	: CUart(uart_nr), deviceName(deviceName), baud_rate(baud_rate)
{
}

void CUartDevice::terminate()
{
	done = true;
	txsem.post();
	CUart::terminate();
}

void CUartDevice::onNotify(smg_uart_t* uart, smg_uart_notify_code_t code)
{
	CUart::onNotify(uart, code);
	switch(code) {
	case UART_NOTIFY_AFTER_OPEN:
		if(baud_rate != 0) {
			uart->baud_rate = baud_rate;
		}
		txsem.post();
		break;

	case UART_NOTIFY_BEFORE_CLOSE:
		txsem.post();
		break;

	case UART_NOTIFY_AFTER_WRITE:
	case UART_NOTIFY_WAIT_TX:
	case UART_NOTIFY_BEFORE_READ:
		break;
	}
}

int CUartDevice::available()
{
	return device ? device->available() : 0;
}

int CUartDevice::readBytes(void* buffer, size_t size)
{
	return device ? device->readBytes(buffer, size) : 0;
}

int CUartDevice::writeBytes(const void* data, size_t size)
{
	return device ? device->writeBytes(data, size) : 0;
}

void* CUartDevice::thread_routine()
{
	while(!done) {
		if(txsem.timedwait(IDLE_SLEEP_MS)) {
			if(uart != nullptr && !device) {
				device.reset(new SerialDevice);
				char res = device->openDevice(deviceName, uart->baud_rate);
				if(res != 1) {
					host_debug_e("UART%u error %d opening serial device '%s'", uart_nr, res, deviceName);
					device.reset();
					break;
				}
				device->DTR(false);
				device->RTS(false);
				host_debug_i("UART%u connected to '%s' @ %u baud", uart_nr, deviceName, uart->baud_rate);
			} else if(uart == nullptr && device) {
				device.reset();
				host_debug_i("Uart #%u device closed", uart_nr);
			}

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

	device.reset();
	return nullptr;
}

} // namespace UartServer
