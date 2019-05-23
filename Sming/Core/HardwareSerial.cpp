/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwareSerial.cpp
 *
 * HardwareSerial based on Espressif Systems' code
 *
 ****/

#include "HardwareSerial.h"
#include <cstdarg>
#include "Platform/System.h"
#include "m_printf.h"

#if ENABLE_CMD_EXECUTOR
#include "../Services/CommandProcessing/CommandExecutor.h"
#endif

HardwareSerial Serial(UART_ID_0);

HardwareSerial::~HardwareSerial()
{
#if ENABLE_CMD_EXECUTOR
	delete commandExecutor;
#endif
}

void HardwareSerial::begin(uint32_t baud, SerialConfig config, SerialMode mode, uint8_t txPin)
{
	end();

	if(uartNr < 0)
		return;

	uart_config cfg = {.uart_nr = (uint8_t)uartNr,
					   .tx_pin = txPin,
					   .mode = static_cast<uart_mode_t>(mode),
					   .options = options,
					   .baudrate = baud,
					   .config = config,
					   .rx_size = rxSize,
					   .tx_size = txSize};
	uart = uart_init_ex(cfg);
	updateUartCallback();
}

void HardwareSerial::end()
{
	if(uart == nullptr) {
		return;
	}

	if(uart_get_debug() == uartNr) {
		uart_set_debug(UART_NO);
	}

	uart_uninit(uart);
	uart = nullptr;
}

size_t HardwareSerial::setRxBufferSize(size_t size)
{
	if(uart) {
		rxSize = uart_resize_rx_buffer(uart, size);
	} else {
		rxSize = size;
	}
	return rxSize;
}

size_t HardwareSerial::setTxBufferSize(size_t size)
{
	if(uart) {
		txSize = uart_resize_tx_buffer(uart, size);
	} else {
		txSize = size;
	}
	return txSize;
}

void HardwareSerial::systemDebugOutput(bool enabled)
{
	if(!uart) {
		return;
	}

	if(enabled) {
		if(uart_tx_enabled(uart)) {
			uart_set_debug(uartNr);
			using namespace std::placeholders;
			m_setPuts(std::bind(&uart_write, uart, _1, _2));
		} else {
			uart_set_debug(UART_NO);
		}
	} else if(uart_get_debug() == uartNr) {
		// Disable system debug messages on this interface
		uart_set_debug(UART_NO);
		// and don't print debugf() data at all
		m_setPuts(nullptr);
	}
}

void HardwareSerial::invokeCallbacks()
{
	uart_disable_interrupts();
	auto status = callbackStatus;
	callbackStatus = 0;
	callbackQueued = false;
	uart_restore_interrupts();

	// Transmit complete ?
	if(bitRead(status, UIFE) && transmitComplete) {
		transmitComplete(*this);
	}

	// RX FIFO Full or RX FIFO Timeout or RX Overflow ?
	if(status & (_BV(UIFF) | _BV(UITO) | _BV(UIOF))) {
		auto receivedChar = uart_peek_last_char(uart);
		if(HWSDelegate) {
			HWSDelegate(*this, receivedChar, uart_rx_available(uart));
		}
#if ENABLE_CMD_EXECUTOR
		if(commandExecutor) {
			int c;
			while((c = uart_read_char(uart)) >= 0) {
				commandExecutor->executorReceive(c);
			}
		}
#endif
	}
}

/*
 * Called via task queue
 */
void HardwareSerial::staticOnStatusChange(uint32_t param)
{
	auto serial = reinterpret_cast<HardwareSerial*>(param);
	if(serial != nullptr) {
		serial->invokeCallbacks();
	}
}

/*
 * Called from uart interrupt handler
 */
void HardwareSerial::staticCallbackHandler(uart_t* uart, uint32_t status)
{
	auto serial = static_cast<HardwareSerial*>(uart_get_callback_param(uart));
	if(serial == nullptr) {
		return;
	}

	serial->callbackStatus |= status;

	// If required, queue a callback
	if((status & serial->statusMask) != 0 && !serial->callbackQueued) {
		System.queueCallback(staticOnStatusChange, uint32_t(serial));
		serial->callbackQueued = true;
	}
}

bool HardwareSerial::updateUartCallback()
{
	uint16_t mask = 0;
#if ENABLE_CMD_EXECUTOR
	if(HWSDelegate || commandExecutor) {
#else
	if(HWSDelegate) {
#endif
		mask |= _BV(UIFF) | _BV(UITO) | _BV(UIOF);
	}

	if(transmitComplete) {
		mask |= _BV(UIFE);
	}

	statusMask = mask;

	setUartCallback(mask == 0 ? nullptr : staticCallbackHandler, this);

	return mask != 0;
}

void HardwareSerial::commandProcessing(bool reqEnable)
{
#if ENABLE_CMD_EXECUTOR
	if(reqEnable) {
		if(!commandExecutor) {
			commandExecutor = new CommandExecutor(this);
		}
	} else {
		delete commandExecutor;
		commandExecutor = nullptr;
	}
	updateUartCallback();
#endif
}
