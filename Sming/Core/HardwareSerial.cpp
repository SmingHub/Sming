/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
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
#include <Services/CommandProcessing/Handler.h>
#endif

HardwareSerial Serial(UART_ID_0);

HardwareSerial::~HardwareSerial()
{
//#if ENABLE_CMD_EXECUTOR
//	delete commandExecutor;
//#endif
}

bool HardwareSerial::begin(uint32_t baud, SerialFormat format, SerialMode mode, uint8_t txPin, uint8_t rxPin)
{
	end();

	if(uartNr < 0) {
		return false;
	}

	smg_uart_config_t cfg = {
		.uart_nr = (uint8_t)uartNr,
		.tx_pin = txPin,
		.rx_pin = rxPin,
		.mode = smg_uart_mode_t(mode),
		.options = options,
		.baudrate = baud,
		.format = smg_uart_format_t(format),
		.rx_size = rxSize,
		.tx_size = txSize,
	};
	uart = smg_uart_init_ex(cfg);
	updateUartCallback();

	return uart != nullptr;
}

void HardwareSerial::end()
{
	if(uart == nullptr) {
		return;
	}

	smg_uart_uninit(uart);
	uart = nullptr;
}

size_t HardwareSerial::setRxBufferSize(size_t size)
{
	if(uart) {
		rxSize = smg_uart_resize_rx_buffer(uart, size);
	} else {
		rxSize = size;
	}
	return rxSize;
}

size_t HardwareSerial::setTxBufferSize(size_t size)
{
	if(uart) {
		txSize = smg_uart_resize_tx_buffer(uart, size);
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
		if(smg_uart_tx_enabled(uart)) {
			smg_uart_set_debug(uartNr);
			m_setPuts(std::bind(&smg_uart_write, uart, _1, _2));
		} else {
			smg_uart_set_debug(UART_NO);
		}
	} else if(smg_uart_get_debug() == uartNr) {
		// Disable system debug messages on this interface
		smg_uart_set_debug(UART_NO);
		// and don't print debugf() data at all
		m_setPuts(nullptr);
	}
}

void HardwareSerial::invokeCallbacks()
{
	(void)smg_uart_disable_interrupts();
	auto status = callbackStatus;
	callbackStatus = 0;
	callbackQueued = false;
	smg_uart_restore_interrupts();

	// Transmit complete ?
	if((status & UART_STATUS_TXFIFO_EMPTY) != 0 && transmitComplete) {
		transmitComplete(*this);
	}

	// RX FIFO Full or RX FIFO Timeout or RX Overflow ?
	if(status & (UART_STATUS_RXFIFO_FULL | UART_STATUS_RXFIFO_TOUT | UART_STATUS_RXFIFO_OVF)) {
		auto receivedChar = smg_uart_peek_last_char(uart);
		if(HWSDelegate) {
			HWSDelegate(*this, receivedChar, smg_uart_rx_available(uart));
		}
//#if ENABLE_CMD_EXECUTOR
//		if(commandExecutor) {
//			int c;
//			while((c = smg_uart_read_char(uart)) >= 0) {
//				commandExecutor->executorReceive(c);
//			}
//		}
//#endif
	}
}

unsigned HardwareSerial::getStatus()
{
	unsigned status = 0;
	unsigned ustat = smg_uart_get_status(uart);
	if(ustat & UART_STATUS_BRK_DET) {
		bitSet(status, eSERS_BreakDetected);
	}

	if(ustat & UART_STATUS_RXFIFO_OVF) {
		bitSet(status, eSERS_Overflow);
	}

	if(ustat & UART_STATUS_FRM_ERR) {
		bitSet(status, eSERS_FramingError);
	}

	if(ustat & UART_STATUS_PARITY_ERR) {
		bitSet(status, eSERS_ParityError);
	}

	return status;
}

/*
 * Called via task queue
 */
void HardwareSerial::staticOnStatusChange(void* param)
{
	auto serial = static_cast<HardwareSerial*>(param);
	if(serial != nullptr) {
		serial->invokeCallbacks();
	}
}

/*
 * Called from uart interrupt handler
 */
void HardwareSerial::staticCallbackHandler(smg_uart_t* uart, uint32_t status)
{
	auto serial = static_cast<HardwareSerial*>(smg_uart_get_callback_param(uart));
	if(serial == nullptr) {
		return;
	}

	serial->callbackStatus |= status;

	// If required, queue a callback
	if((status & serial->statusMask) != 0 && !serial->callbackQueued) {
		System.queueCallback(staticOnStatusChange, serial);
		serial->callbackQueued = true;
	}
}

bool HardwareSerial::updateUartCallback()
{
	uint16_t mask = 0;
//#if ENABLE_CMD_EXECUTOR
//	if(HWSDelegate || commandExecutor) {
//#else
	if(HWSDelegate) {
//#endif
		mask |= UART_STATUS_RXFIFO_FULL | UART_STATUS_RXFIFO_TOUT | UART_STATUS_RXFIFO_OVF;
	}

	if(transmitComplete) {
		mask |= UART_STATUS_TXFIFO_EMPTY;
	}

	statusMask = mask;

	setUartCallback(mask == 0 ? nullptr : staticCallbackHandler, this);

	return mask != 0;
}

void HardwareSerial::commandProcessing(bool reqEnable)
{
//#if ENABLE_CMD_EXECUTOR
//	if(reqEnable) {
//		if(!commandExecutor) {
//			commandExecutor = new CommandExecutor(this);
//		}
//	} else {
//		delete commandExecutor;
//		commandExecutor = nullptr;
//	}
//	updateUartCallback();
//#endif
}
