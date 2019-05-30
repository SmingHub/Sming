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

#include <espinc/peri.h>
#include <Data/Buffer/SerialBuffer.h>
#include <BitManipulations.h>

const unsigned IDLE_SLEEP_MS = 100;

unsigned CUartServer::portBase = 10000;

static CUartServer* uartServers[UART_COUNT];

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
}

void CUartServer::shutdown()
{
	for(unsigned i = 0; i < UART_COUNT; ++i) {
		auto& server = uartServers[i];
		delete server;
		server = nullptr;
	}
}

CUartServer::~CUartServer()
{
	close();
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

	int space = uart->rx_buffer->getFreeSpace();
	int read = std::min(space, avail);
	if(read == 0) {
		return 0;
	}

	char buffer[read];
	read = socket->recv(buffer, read);
	if(read > 0) {
		for(int i = 0; i < read; ++i) {
			uart->rx_buffer->writeChar(buffer[i]);
		}
		space -= read;
		if(space == 0) {
			bitSet(uart->status, UIFF);
		} else {
			bitSet(uart->status, UITO);
		}
	}

	return read;
}

int CUartServer::serviceWrite()
{
	if(!uart_tx_enabled(uart)) {
		return 0;
	}

	int result = 0;
	size_t avail;
	void* data;
	auto txbuf = uart->tx_buffer;
	while((avail = txbuf->getReadData(data)) != 0) {
		int sent = socket->send(data, avail);
		if(sent < 0) {
			hostmsg("Uart send returned %d", sent);
			result = sent;
			break;
		}
		txbuf->skipRead(sent);
		result += sent;
	}

	if(txbuf->isEmpty()) {
		bitSet(uart->status, UIFE);
	} else {
		txsem.post();
	}

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
				auto status = uart->status;
				uart->status = 0;
				if(status != 0 && uart->callback != nullptr) {
					uart->callback(uart, status);
				}
			}
		}

		socket->close();
		hostmsg("Uart #%u socket closed", uart_nr);
	}

	return nullptr;
}
