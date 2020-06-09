/*
 uart.cpp - esp8266 UART HAL

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 @author 2018 mikee47 <mike@sillyhouse.net>

 Additional features to support flexible transmit buffering and callbacks

 */

#include <driver/uart.h>
#include <SerialBuffer.h>
#include <BitManipulations.h>
#include <Clock.h>

/*
 * Parameters relating to RX FIFO and buffer thresholds
 *
 * 'headroom' is the number of characters which may be received before a receive overrun
 * condition occurs and data is lost.
 *
 * For the hardware FIFO, data is processed via interrupt so the headroom can be fairly small.
 * The greater the headroom, the more interrupts will be generated thus reducing efficiency.
 */
#define RX_FIFO_FULL_THRESHOLD 120									  ///< UIFF interrupt when FIFO bytes > threshold
#define RX_FIFO_HEADROOM (UART_RX_FIFO_SIZE - RX_FIFO_FULL_THRESHOLD) ///< Chars between UIFF and UIOF
/*
 * Using a buffer, data is typically processed via task callback so requires additional time.
 * This figure is set to a nominal default which should provide robust operation for most situations.
 * It can be adjusted if necessary via the rx_headroom parameter.
*/
#define DEFAULT_RX_HEADROOM (32 - RX_FIFO_HEADROOM)

static int s_uart_debug_nr = UART_NO;

// Keep track of interrupt enable state for each UART
static uint8_t isrMask;
// Keep a reference to all created UARTS - required because they share an ISR
static smg_uart_t* uartInstances[UART_COUNT];

// Registered port callback functions
static smg_uart_notify_callback_t notifyCallbacks[UART_COUNT];

/** @brief Invoke a port callback, if one has been registered
 *  @param uart
 *  @param code
 */
static void notify(smg_uart_t* uart, smg_uart_notify_code_t code)
{
	auto callback = notifyCallbacks[uart->uart_nr];
	if(callback != nullptr) {
		callback(uart, code);
	}
}

__forceinline static bool smg_uart_isr_enabled(uint8_t nr)
{
	return bitRead(isrMask, nr);
}

smg_uart_t* smg_uart_get_uart(uint8_t uart_nr)
{
	return (uart_nr < UART_COUNT) ? uartInstances[uart_nr] : nullptr;
}

uint8_t smg_uart_disable_interrupts()
{
	return isrMask;
}

void smg_uart_restore_interrupts()
{
}

bool smg_uart_set_notify(unsigned uart_nr, smg_uart_notify_callback_t callback)
{
	if(uart_nr >= UART_COUNT) {
		return false;
	}

	notifyCallbacks[uart_nr] = callback;
	return true;
}

void smg_uart_set_callback(smg_uart_t* uart, smg_uart_callback_t callback, void* param)
{
	if(uart != nullptr) {
		uart->callback = nullptr; // In case interrupt fires between setting param and callback
		uart->param = param;
		uart->callback = callback;
	}
}

static bool realloc_buffer(SerialBuffer*& buffer, size_t new_size)
{
	if(buffer != nullptr) {
		if(new_size == 0) {
			(void)smg_uart_disable_interrupts();
			delete buffer;
			buffer = nullptr;
			smg_uart_restore_interrupts();
			return true;
		}

		return buffer->resize(new_size) == new_size;
	}

	if(new_size == 0) {
		return true;
	}

	auto new_buf = new SerialBuffer;
	if(new_buf != nullptr && new_buf->resize(new_size) == new_size) {
		buffer = new_buf;
		return true;
	}

	delete new_buf;
	return false;
}

size_t smg_uart_resize_rx_buffer(smg_uart_t* uart, size_t new_size)
{
	if(smg_uart_rx_enabled(uart)) {
		realloc_buffer(uart->rx_buffer, new_size);
	}
	return smg_uart_rx_buffer_size(uart);
}

size_t smg_uart_rx_buffer_size(smg_uart_t* uart)
{
	return uart != nullptr && uart->rx_buffer != nullptr ? uart->rx_buffer->getSize() : 0;
}

size_t smg_uart_resize_tx_buffer(smg_uart_t* uart, size_t new_size)
{
	if(smg_uart_tx_enabled(uart)) {
		realloc_buffer(uart->tx_buffer, new_size);
	}
	return smg_uart_tx_buffer_size(uart);
}

size_t smg_uart_tx_buffer_size(smg_uart_t* uart)
{
	return uart != nullptr && uart->tx_buffer != nullptr ? uart->tx_buffer->getSize() : 0;
}

int smg_uart_peek_char(smg_uart_t* uart)
{
	return uart != nullptr && uart->rx_buffer ? uart->rx_buffer->peekChar() : -1;
}

int smg_uart_rx_find(smg_uart_t* uart, char c)
{
	if(uart == nullptr || uart->rx_buffer == nullptr) {
		return -1;
	}

	return uart->rx_buffer->find(c);
}

int smg_uart_peek_last_char(smg_uart_t* uart)
{
	return uart != nullptr && uart->rx_buffer != nullptr ? uart->rx_buffer->peekLastChar() : -1;
}

size_t smg_uart_read(smg_uart_t* uart, void* buffer, size_t size)
{
	if(!smg_uart_rx_enabled(uart) || buffer == nullptr || size == 0) {
		return 0;
	}

	notify(uart, UART_NOTIFY_BEFORE_READ);

	size_t read = 0;

	auto buf = static_cast<uint8_t*>(buffer);

	// First read data from RX buffer if in use
	if(uart->rx_buffer != nullptr) {
		while(read < size && !uart->rx_buffer->isEmpty())
			buf[read++] = uart->rx_buffer->readChar();
	}

	return read;
}

size_t smg_uart_rx_available(smg_uart_t* uart)
{
	if(!smg_uart_rx_enabled(uart)) {
		return 0;
	}

	(void)smg_uart_disable_interrupts();

	size_t avail = 0;

	if(uart->rx_buffer != nullptr) {
		avail += uart->rx_buffer->available();
	}

	smg_uart_restore_interrupts();

	return avail;
}

void smg_uart_start_isr(smg_uart_t* uart)
{
	if(!bitRead(isrMask, uart->uart_nr)) {
		bitSet(isrMask, uart->uart_nr);
	}
}

size_t smg_uart_write(smg_uart_t* uart, const void* buffer, size_t size)
{
	if(!smg_uart_tx_enabled(uart) || buffer == nullptr || size == 0) {
		return 0;
	}

	size_t written = 0;

	auto buf = static_cast<const uint8_t*>(buffer);

	while(written < size) {
		if(uart->tx_buffer != nullptr) {
			while(written < size && uart->tx_buffer->writeChar(buf[written])) {
				++written;
			}
		}

		notify(uart, UART_NOTIFY_AFTER_WRITE);

		if(!bitRead(uart->options, UART_OPT_TXWAIT)) {
			break;
		}
	}

	return written;
}

size_t smg_uart_tx_free(smg_uart_t* uart)
{
	if(!smg_uart_tx_enabled(uart)) {
		return 0;
	}

	(void)smg_uart_disable_interrupts();

	size_t space = 0;
	if(uart->tx_buffer != nullptr) {
		space += uart->tx_buffer->getFreeSpace();
	}

	smg_uart_restore_interrupts();

	return space;
}

void smg_uart_wait_tx_empty(smg_uart_t* uart)
{
	if(!smg_uart_tx_enabled(uart)) {
		return;
	}

	notify(uart, UART_NOTIFY_WAIT_TX);

	if(uart->tx_buffer != nullptr) {
		while(!uart->tx_buffer->isEmpty()) {
			delay(0);
		}
	}
}

void smg_uart_set_break(smg_uart_t* uart, bool state)
{
	//	uart = get_physical(uart);
	//	if(uart != nullptr) {
	//		bitWrite(USC0(uart->uart_nr), UCBRK, state);
	//	}
}

uint8_t smg_uart_get_status(smg_uart_t* uart)
{
	uint8_t status = 0;
	//	if(uart != nullptr) {
	//		uart_disable_interrupts();
	//		// Get break/overflow flags from actual uart (physical or otherwise)
	//		status = uart->status & (_BV(UIBD) | _BV(UIOF));
	//		uart->status = 0;
	//		// Read raw status register directly from real uart, masking out non-error bits
	//		uart = get_physical(uart);
	//		if(uart != nullptr) {
	//			status |= USIR(uart->uart_nr) & (_BV(UIBD) | _BV(UIOF) | _BV(UIFR) | _BV(UIPE));
	//			// Clear errors
	//			USIC(uart->uart_nr) = status;
	//		}
	//		uart_restore_interrupts();
	//	}
	return status;
}

void smg_uart_flush(smg_uart_t* uart, smg_uart_mode_t mode)
{
	if(uart == nullptr) {
		return;
	}

	bool flushRx = mode != UART_TX_ONLY && uart->mode != UART_TX_ONLY;
	bool flushTx = mode != UART_RX_ONLY && uart->mode != UART_RX_ONLY;

	(void)smg_uart_disable_interrupts();
	if(flushRx && uart->rx_buffer != nullptr) {
		uart->rx_buffer->clear();
	}

	if(flushTx && uart->tx_buffer != nullptr) {
		uart->tx_buffer->clear();
	}

	smg_uart_restore_interrupts();
}

uint32_t smg_uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate)
{
	//	if(!is_physical(uart_nr) || baud_rate == 0) {
	//		return 0;
	//	}
	//
	//	uint32_t clkdiv = ESP8266_CLOCK / baud_rate;
	//	USD(uart_nr) = clkdiv;
	//	// Return the actual baud rate in use
	//	baud_rate = clkdiv ? ESP8266_CLOCK / clkdiv : 0;
	return baud_rate;
}

uint32_t smg_uart_set_baudrate(smg_uart_t* uart, uint32_t baud_rate)
{
	if(uart == nullptr) {
		return 0;
	}

	baud_rate = smg_uart_set_baudrate_reg(uart->uart_nr, baud_rate);
	// Store the actual baud rate in use
	uart->baud_rate = baud_rate;
	return baud_rate;
}

uint32_t smg_uart_get_baudrate(smg_uart_t* uart)
{
	return (uart == nullptr) ? 0 : uart->baud_rate;
}

smg_uart_t* smg_uart_init_ex(const smg_uart_config& cfg)
{
	// Already initialised?
	if(smg_uart_get_uart(cfg.uart_nr) != nullptr) {
		return nullptr;
	}

	auto uart = new smg_uart_t;
	if(uart == nullptr) {
		return nullptr;
	}

	memset(uart, 0, sizeof(smg_uart_t));
	uart->uart_nr = cfg.uart_nr;
	uart->mode = cfg.mode;
	uart->options = cfg.options;
	uart->tx_pin = 255;
	uart->rx_pin = 255;
	uart->rx_headroom = DEFAULT_RX_HEADROOM;

	auto rxBufferSize = cfg.rx_size;
	auto txBufferSize = cfg.tx_size;

	// Virtual uart requires a minimum RAM buffer
	rxBufferSize += UART_RX_FIFO_SIZE;
	txBufferSize += UART_TX_FIFO_SIZE;

	switch(cfg.uart_nr) {
	case UART0:
	case UART2:
		if(smg_uart_rx_enabled(uart) && !realloc_buffer(uart->rx_buffer, rxBufferSize)) {
			delete uart;
			return nullptr;
		}

		if(smg_uart_tx_enabled(uart) && !realloc_buffer(uart->tx_buffer, txBufferSize)) {
			delete uart->rx_buffer;
			delete uart;
			return nullptr;
		}

		if(cfg.uart_nr == UART2) {
			break;
		}

		// OK, buffers allocated so setup hardware
		smg_uart_detach(cfg.uart_nr);

		break;

	case UART1:
		// Note: uart_interrupt_handler does not support RX on UART 1
		if(uart->mode == UART_RX_ONLY) {
			delete uart;
			return nullptr;
		}
		uart->mode = UART_TX_ONLY;

		// Transmit buffer optional
		if(!realloc_buffer(uart->tx_buffer, txBufferSize)) {
			delete uart;
			return nullptr;
		}

		// Setup hardware
		smg_uart_detach(cfg.uart_nr);
		break;

	default:
		// big fail!
		delete uart;
		return nullptr;
	}

	smg_uart_set_baudrate(uart, cfg.baudrate);
	smg_uart_flush(uart);
	uartInstances[cfg.uart_nr] = uart;
	smg_uart_start_isr(uart);

	notify(uart, UART_NOTIFY_AFTER_OPEN);

	return uart;
}

void smg_uart_uninit(smg_uart_t* uart)
{
	if(uart == nullptr) {
		return;
	}

	notify(uart, UART_NOTIFY_BEFORE_CLOSE);

	smg_uart_stop_isr(uart);
	// If debug output being sent to this UART, disable it
	if(uart->uart_nr == s_uart_debug_nr) {
		smg_uart_set_debug(UART_NO);
	}

	delete uart->rx_buffer;
	delete uart->tx_buffer;
	delete uart;
}

smg_uart_t* smg_uart_init(uint8_t uart_nr, uint32_t baudrate, uint32_t config, smg_uart_mode_t mode, uint8_t tx_pin, size_t rx_size,
				  size_t tx_size)
{
	smg_uart_config cfg = {.uart_nr = uart_nr,
					   .tx_pin = tx_pin,
					   .mode = mode,
					   .options = _BV(UART_OPT_TXWAIT),
					   .baudrate = baudrate,
					   .config = config,
					   .rx_size = rx_size,
					   .tx_size = tx_size};
	return smg_uart_init_ex(cfg);
}

void smg_uart_swap(smg_uart_t* uart, int tx_pin)
{
}

void smg_uart_set_tx(smg_uart_t* uart, int tx_pin)
{
}

void smg_uart_set_pins(smg_uart_t* uart, int tx, int rx)
{
}

void smg_uart_debug_putc(char c)
{
	smg_uart_t* uart = smg_uart_get_uart(s_uart_debug_nr);
	if(uart != nullptr) {
		smg_uart_write_char(uart, c);
	}
}

void smg_uart_set_debug(int uart_nr)
{
	s_uart_debug_nr = uart_nr;
	system_set_os_print(uart_nr >= 0);
	ets_install_putc1(smg_uart_debug_putc);
}

int smg_uart_get_debug()
{
	return s_uart_debug_nr;
}

void smg_uart_detach(int uart_nr)
{
	bitClear(isrMask, uart_nr);
}

void smg_uart_detach_all()
{
	//	uart_disable_interrupts();
	//	for(unsigned uart_nr = 0; uart_nr < UART_PHYSICAL_COUNT; ++uart_nr) {
	//		USC1(uart_nr) = 0;
	//		USIC(uart_nr) = 0xffff;
	//		USIE(uart_nr) = 0;
	//	}
	//	isrMask = 0;
}
