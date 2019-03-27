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
#include "System.h"
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

void HardwareSerial::staticOnTransmitComplete(uint32_t param)
{
	auto serial = reinterpret_cast<HardwareSerial*>(param);
	if(serial->transmitComplete) {
		serial->transmitComplete(*serial);
	}
}

void HardwareSerial::staticOnReceive(uint32_t param)
{
	auto serial = reinterpret_cast<HardwareSerial*>(param);
	auto receivedChar = uart_peek_last_char(serial->uart);
	if(serial->HWSDelegate) {
		serial->HWSDelegate(*serial, receivedChar, uart_rx_available(serial->uart));
	}
#if ENABLE_CMD_EXECUTOR
	if(serial->commandExecutor) {
		serial->commandExecutor->executorReceive(receivedChar);
	}
#endif
}

void HardwareSerial::callbackHandler(uint32_t status)
{
	// Transmit complete ?
	if(bitRead(status, UIFE) && transmitComplete) {
		System.queueCallback(staticOnTransmitComplete, uint32_t(this));
	}

	// RX FIFO Full or RX FIFO Timeout ?
	if((status & (_BV(UIFF) | _BV(UITO))) != 0) {
#if ENABLE_CMD_EXECUTOR
		if(HWSDelegate || commandExecutor) {
#else
		if(HWSDelegate) {
#endif

			System.queueCallback(staticOnReceive, uint32_t(this));
		}
	}
}

void HardwareSerial::staticCallbackHandler(uart_t* uart, uint32_t status)
{
	auto serial = reinterpret_cast<HardwareSerial*>(uart_get_callback_param(uart));
	if(serial) {
		serial->callbackHandler(status);
	}
}

bool HardwareSerial::updateUartCallback()
{
	if(uart == nullptr) {
		return false;
	}

#if ENABLE_CMD_EXECUTOR
	if(HWSDelegate || transmitComplete || commandExecutor) {
#else
	if(HWSDelegate || transmitComplete) {
#endif
		setUartCallback(staticCallbackHandler, this);
		return true;
	} else {
		setUartCallback(nullptr, nullptr);
		return false;
	}
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
