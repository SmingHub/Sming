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


/**
 *  UART GPIOs
 *
 * UART0 TX: 1 or 2
 * UART0 RX: 3
 *
 * UART0 SWAP TX: 15
 * UART0 SWAP RX: 13
 *
 *
 * UART1 TX: 7 (NC) or 2
 * UART1 RX: 8 (NC)
 *
 * UART1 SWAP TX: 11 (NC)
 * UART1 SWAP RX: 6 (NC)
 *
 * NC = Not Connected to Module Pads --> No Access
 *
 */
#include "Clock.h"
#include "Digital.h"

#include "espinc/uart.h"
#include "espinc/peri.h"

#include "SerialBuffer.h"

static int s_uart_debug_nr = UART0;

// Get number of characters in receive FIFO
#define UART_RXCOUNT(nr) ((USS(nr) >> USRXC) & 0x7f)

// Get number of characters in transmit FIFO
#define UART_TXCOUNT(nr) (USS(nr) >> USTXC)

// Return true if transmit FIFO is full
#define UART_TXFULL(nr) (UART_TXCOUNT(nr) >= 0x7f)

// Keep track of interrupt enable state for each UART
static uint8_t isrMask;
// Keep a reference to all created UARTS - required because they share an ISR
static uart_t* uartInstances[UART_COUNT];

#define UART_ISR_ENABLED(nr) (isrMask & _BV(nr))

uart_t* IRAM_ATTR uart_get_uart(uint8_t uart_nr)
{
	return (uart_nr < UART_COUNT) ? uartInstances[uart_nr] : nullptr;
}

/** @brief disable interrupts and return current interrupt state
 *  @retval state non-zero if any UART interrupts were active
 */
__forceinline static uint8_t uart_disable_interrupts()
{
	ETS_UART_INTR_DISABLE();
	return isrMask;
}

/** @brief re-enable interrupts after calling uart_disable_interrupts()
 */
__forceinline static void uart_restore_interrupts()
{
	if(isrMask)
		ETS_UART_INTR_ENABLE();
}

void IRAM_ATTR uart_set_callback(uart_t* uart, uart_callback_t callback, void* param)
{
	if(uart) {
		uart_disable_interrupts();
		uart->callback = callback;
		uart->param = param;
		uart_restore_interrupts();
	}
}

static bool realloc_buffer(SerialBuffer*& buffer, size_t new_size)
{
	if(buffer) {
		if (new_size == 0) {
			uart_disable_interrupts();
			delete buffer;
			buffer = nullptr;
			uart_restore_interrupts();
			return true;
		}

		return buffer->resize(new_size) == new_size;
	}

	if (new_size == 0)
		return true;

	auto new_buf = new SerialBuffer;
	if(new_buf && new_buf->resize(new_size)) {
		buffer = new_buf;
		return true;
	}

	delete new_buf;
	return false;
}

size_t uart_resize_rx_buffer(uart_t* uart, size_t new_size)
{
	if (uart_rx_enabled(uart))
		realloc_buffer(uart->rx_buffer, new_size);
	return uart_rx_buffer_size(uart);
}

size_t uart_rx_buffer_size(uart_t* uart)
{
	return uart && uart->rx_buffer ? uart->rx_buffer->getSize() : 0;
}

size_t uart_resize_tx_buffer(uart_t* uart, size_t new_size)
{
	if (uart_tx_enabled(uart))
		realloc_buffer(uart->tx_buffer, new_size);
	return uart_tx_buffer_size(uart);
}

size_t uart_tx_buffer_size(uart_t* uart)
{
	return uart && uart->tx_buffer ? uart->tx_buffer->getSize() : 0;
}

int uart_peek_char(uart_t* uart)
{
	return uart && uart->rx_buffer ? uart->rx_buffer->peekChar() : -1;
}

int uart_rx_find(uart_t* uart, char c)
{
	if (!uart || !uart->rx_buffer)
		return -1;

	return uart->rx_buffer->find(c);
}

int uart_peek_last_char(uart_t* uart)
{
	return uart && uart->rx_buffer ? uart->rx_buffer->peekLastChar() : -1;
}

size_t uart_read(uart_t* uart, void* buffer, size_t size)
{
	if(!uart_rx_enabled(uart) || !buffer || !size)
		return 0;

	size_t read = 0;

	auto buf = static_cast<uint8_t*>(buffer);

	// If RX buffer not in use or it's empty then read directly from hardware FIFO
	if(uart->rx_buffer)
		while(read < size && !uart->rx_buffer->isEmpty())
			buf[read++] = uart->rx_buffer->readChar();

	while(read < size && UART_RXCOUNT(uart->uart_nr) != 0)
		buf[read++] = USF(uart->uart_nr);

	// FIFO full may have been disabled if buffer overflowed, re-enabled it now
	USIE(uart->uart_nr) |= _BV(UIFF) | _BV(UITO);

	return read;
}

size_t uart_rx_available(uart_t* uart)
{
	if(!uart_rx_enabled(uart))
		return 0;

	size_t avail = UART_RXCOUNT(uart->uart_nr);

	if(uart->rx_buffer)
		avail += uart->rx_buffer->available();

	return avail;
}


/**
 * @brief service interrupts for a UART
 * @param uart_nr identifies which UART to check
 * @param uart the allocated uart structure, which may be NULL if port hasn't been setup
 */
static void IRAM_ATTR handle_uart_interrupt(uint8_t uart_nr, uart_t* uart)
{
	uint32_t usis = USIS(uart_nr);

	// If status is clear there's no interrupt to service on this UART
	if (usis == 0)
		return;

	// Clear all status before proceeeding
	USIC(uart_nr) = usis;

	/*
	 * If we haven't asked for interrupts on this UART, then disable all interrupt sources for it.
	 *
	 * This happens at startup where we've only initialised one of the UARTS. For example, we initialise
	 * UART1 for debug output but leave UART0 alone. However, the SDK has enabled some interrupt sources
	 * which we're not expecting.
	 *
	 * @todo Call uart_detach() for all UARTs at power-on.
	 */
	if (!uart || !UART_ISR_ENABLED(uart_nr)) {
		USIE(uart_nr) = 0;
		return;
	}


	// Rx FIFO full or timeout
	if (usis & (_BV(UIFF) | _BV(UITO))) {
		size_t read = 0;

		// Read as much data as possible from the RX FIFO into buffer
		if(uart->rx_buffer) {
			size_t space = uart->rx_buffer->getFreeSpace();
			while(space-- && UART_RXCOUNT(uart_nr) != 0) {
				uart->rx_buffer->writeChar(USF(uart_nr));
				++read;
			}
		}

		// We cleared status flags  above, but this one gets re-set almost immediately so clear it again now
		USIC(uart_nr) |= _BV(UITO);

		/*
		 * If the FIFO is full and we didn't read any of the data then need to mask the interrupt out or it'll recur.
		 * The interrupt gets re-enabled by a call to uart_read() or uart_flush()
		 */
		if (read == 0)
			USIE(uart_nr) &= ~(_BV(UIFF) | _BV(UITO));
	}


	// Unless we replenish TX FIFO, disable after handling interrupt
	bool tx_fifo_empty = (usis & _BV(UIFE)) != 0;

	if (tx_fifo_empty) {
		// Dump as much data as we can from buffer into the TX FIFO
		if(uart->tx_buffer && !uart->tx_buffer->isEmpty()) {
			size_t avail = uart->tx_buffer->available();
			while(avail-- && !UART_TXFULL(uart_nr))
				USF(uart_nr) = uart->tx_buffer->readChar();

			// We've topped up TX FIFO so defer callback until next time
			if (UART_TXCOUNT(uart_nr) != 0) {
				usis &= ~_BV(UIFE);
				tx_fifo_empty = false;
			}
		}
	}

	/*
	 * If TX FIFO remains empty then we must disable TX FIFO EMPTY interrupt to stop it recurring.
	 * The interrupt gets re-enabled by uart_write()
	 */
	if (tx_fifo_empty)
		USIE(uart_nr) &= ~_BV(UIFE);

	if(usis && uart->callback)
		uart->callback(uart, usis);
}


/** @brief UART interrupt service routine
 *  @note both UARTS share the same ISR, although UART1 only supports transmit
 */
static void IRAM_ATTR uart_isr(void* arg)
{
	handle_uart_interrupt(UART0, uartInstances[UART0]);
	handle_uart_interrupt(UART1, uartInstances[UART1]);
}


void uart_start_isr(uart_t* uart)
{
	if (!uart || uart->uart_nr >= UART_COUNT)
		return;

	uint32_t usc1 = 0;
	uint32_t usie = 0;

	if(uart_rx_enabled(uart)) {
		/* UCFFT: RX FIFO Full Threshold
		 * UCTOT: RX TimeOut Treshold
		 * UCTOE: RX TimeOut Enable
		 */
		usc1 = (127 << UCFFT) | (0x02 << UCTOT) | _BV(UCTOE);
		usie = _BV(UIFF) | _BV(UIFR) | _BV(UITO);
	}

	if (uart_tx_enabled(uart)) {
		/*
		 * We can interrupt when TX FIFO is empty; at 1Mbit that gives us 800 CPU
		 * cycles before the last character has actually gone over the wire. Even if
		 * a gap occurs it is unlike to cause any problems. It also makes the callback
		 * more useful, for example if using it for RS485 we'd then want to reverse
		 * transfer direction and begin waiting for a response.
		 */

		// TX FIFO empty threshold
		usc1 |= (0 << UCFET);
		// TX FIFO empty interrupt only gets enabled via uart_write function()
	}

	USC1(uart->uart_nr) = usc1;
	USIC(uart->uart_nr) = 0xffff;
	USIE(uart->uart_nr) = usie;

	uint8_t oldmask = isrMask;

	isrMask |= _BV(uart->uart_nr);

	if(oldmask == 0) {
		ETS_UART_INTR_DISABLE();
		ETS_UART_INTR_ATTACH(uart_isr, nullptr);
		ETS_UART_INTR_ENABLE();
	}
}

void IRAM_ATTR uart_stop_isr(uart_t* uart)
{
	if (uart)
		uart_detach(uart->uart_nr);
}


size_t uart_write(uart_t* uart, const void* buffer, size_t size)
{
	if(!uart_tx_enabled(uart) || !buffer || !size)
		return 0;

	size_t written = 0;

	auto buf = static_cast<const uint8_t*>(buffer);

	for (;;) {
		// If TX buffer not in use or it's empty then write directly to hardware FIFO
		if(!uart->tx_buffer || uart->tx_buffer->isEmpty())
			while(written < size && !UART_TXFULL(uart->uart_nr))
				USF(uart->uart_nr) = buf[written++];

		// Write any remaining data into buffer
		if(uart->tx_buffer) {
			while(written < size && uart->tx_buffer->writeChar(buf[written]))
				++written;
		}


		if (written >= size || !(uart->options & _BV(UART_OPT_TXWAIT)))
			break;
	}

	// Enable TX FIFO EMPTY interrupt
	if (written)
		USIE(uart->uart_nr) |= _BV(UIFE);

	return written;
}

size_t uart_tx_free(uart_t* uart)
{
	if(!uart_tx_enabled(uart))
		return 0;

	size_t space = UART_TX_FIFO_SIZE - UART_TXCOUNT(uart->uart_nr);
	if(uart->tx_buffer)
		space += uart->tx_buffer->getFreeSpace();
	return space;
}

void uart_wait_tx_empty(uart_t* uart)
{
	if(!uart_tx_enabled(uart))
		return;

	if (uart->tx_buffer)
		while (!uart->tx_buffer->isEmpty()) {
			delay(0);
		}

	while(UART_TXCOUNT(uart->uart_nr) != 0)
		delay(0);
}

void uart_flush(uart_t* uart)
{
	if(!uart)
		return;

	uart_disable_interrupts();
	if(uart->rx_buffer)
		uart->rx_buffer->clear();

	if(uart->tx_buffer)
		uart->tx_buffer->clear();

	// Prevend TX FIFO EMPTY interrupts - don't need them until uart_write is called again
	USIE(uart->uart_nr) &= ~_BV(UIFE);

	// If receive overflow occurred then these interrupts will be masked
	if(uart_rx_enabled(uart))
		USIE(uart->uart_nr) |= _BV(UIFF) | _BV(UITO);

	uint32_t tmp = _BV(UCRXRST) | _BV(UCTXRST);
	USC0(uart->uart_nr) |= tmp;
	USC0(uart->uart_nr) &= ~tmp;

	uart_restore_interrupts();
}

uint32_t uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate)
{
	if(uart_nr < UART0 || uart_nr > UART1 || baud_rate == 0)
		return 0;

	uint32_t clkdiv = ESP8266_CLOCK / baud_rate;
	USD(uart_nr) = clkdiv;
	// Return the actual baud rate in use
	baud_rate = clkdiv ? ESP8266_CLOCK / clkdiv : 0;
	return baud_rate;
}

uint32_t uart_set_baudrate(uart_t* uart, uint32_t baud_rate)
{
	if(!uart)
		return 0;

	baud_rate = uart_set_baudrate_reg(uart->uart_nr, baud_rate);
	// Store the actual baud rate in use
	uart->baud_rate = baud_rate;
	return baud_rate;
}

uart_t* uart_init_ex(const uart_config& cfg)
{
	// Already initialised?
	if (uart_get_uart(cfg.uart_nr))
		return nullptr;

	auto uart = new uart_t;
	if(!uart)
		return nullptr;

	memset(uart, 0, sizeof(uart_t));
	uart->uart_nr = cfg.uart_nr;
	uart->mode = cfg.mode;
	uart->options = cfg.options;

	switch(cfg.uart_nr) {
	case UART0:
		uart->rx_pin = uart_rx_enabled(uart) ? 3 : 255;

		if(uart_rx_enabled(uart))
			if (!realloc_buffer(uart->rx_buffer, cfg.rx_size)) {
				delete uart;
				return nullptr;
			}

		if(uart_tx_enabled(uart)) {
			if (!realloc_buffer(uart->tx_buffer, cfg.tx_size)) {
				delete uart->rx_buffer;
				delete uart;
				return nullptr;
			}
		}

		// OK, buffers allocated so setup hardware
		uart_detach(cfg.uart_nr);

		if(uart_rx_enabled(uart)) {
			pinMode(uart->rx_pin, SPECIAL);
		}

		if(uart_tx_enabled(uart)) {
			if(cfg.tx_pin == 2) {
				uart->tx_pin = 2;
				pinMode(uart->tx_pin, FUNCTION_4);
			} else {
				uart->tx_pin = 1;
				pinMode(uart->tx_pin, FUNCTION_0);
			}
		} else
			uart->tx_pin = 255;

		IOSWAP &= ~(1 << IOSWAPU0);
		break;

	case UART1:
		// Note: uart_interrupt_handler does not support RX on UART 1
		if(uart->mode == UART_RX_ONLY) {
			delete uart;
			return nullptr;
		}
		uart->mode = UART_TX_ONLY;
		uart->rx_pin = 255;
		// GPIO7 as TX not possible! See GPIO pins used by UART
		uart->tx_pin = 2;

		// Transmit buffer optional
		if(cfg.tx_size)
			if (!realloc_buffer(uart->tx_buffer, cfg.tx_size)) {
				delete uart;
				return nullptr;
			}

		// Setup hardware
		uart_detach(cfg.uart_nr);
		pinMode(uart->tx_pin, SPECIAL);
		break;

	default:
		// big fail!
		delete uart;
		return nullptr;
	}

	uart_set_baudrate(uart, cfg.baudrate);
	USC0(cfg.uart_nr) = cfg.config;
	uart_flush(uart);
	uartInstances[cfg.uart_nr] = uart;
	uart_start_isr(uart);

	return uart;
}

void uart_uninit(uart_t* uart)
{
	if(!uart)
		return;

	uart_stop_isr(uart);
	// If debug output being sent to this UART, disable it
	if (uart->uart_nr == s_uart_debug_nr)
		uart_set_debug(UART_NO);

	switch(uart->rx_pin) {
	case 3:
		pinMode(3, INPUT);
		break;

	case 13:
		pinMode(13, INPUT);
		break;
	}

	switch(uart->tx_pin) {
	case 1:
		pinMode(1, INPUT);
		break;

	case 2:
		pinMode(2, INPUT);
		break;

	case 15:
		pinMode(15, INPUT);
		break;
	}

	delete uart->rx_buffer;
	delete uart->tx_buffer;
	delete uart;
}


uart_t* uart_init(uint8_t uart_nr, uint32_t baudrate, uint32_t config, uart_mode_t mode, uint8_t tx_pin, size_t rx_size, size_t tx_size)
{
	uart_config cfg = {
		.uart_nr = uart_nr,
		.tx_pin = tx_pin,
		.mode = mode,
		.options = _BV(UART_OPT_TXWAIT),
		.baudrate = baudrate,
		.config = config,
		.rx_size = rx_size,
		.tx_size = tx_size
	};
	return uart_init_ex(cfg);
}


void uart_swap(uart_t* uart, int tx_pin)
{
	if(!uart)
		return;

	switch(uart->uart_nr) {
	case UART0:
		if(((uart->tx_pin == 1 || uart->tx_pin == 2) && uart_tx_enabled(uart)) ||
		   (uart->rx_pin == 3 && uart_rx_enabled(uart))) {
			if(uart_tx_enabled(uart)) {
				pinMode(uart->tx_pin, INPUT);
				uart->tx_pin = 15;
			}

			if(uart_rx_enabled(uart)) {
				pinMode(uart->rx_pin, INPUT);
				uart->rx_pin = 13;
			}

			if(uart_tx_enabled(uart))
				pinMode(uart->tx_pin, FUNCTION_4);

			if(uart_rx_enabled(uart))
				pinMode(uart->rx_pin, FUNCTION_4);

			IOSWAP |= (1 << IOSWAPU0);
		} else {
			if(uart_tx_enabled(uart)) {
				pinMode(uart->tx_pin, INPUT);
				uart->tx_pin = (tx_pin == 2) ? 2 : 1;
			}

			if(uart_rx_enabled(uart)) {
				pinMode(uart->rx_pin, INPUT);
				uart->rx_pin = 3;
			}

			if(uart_tx_enabled(uart))
				pinMode(uart->tx_pin, (tx_pin == 2) ? FUNCTION_4 : SPECIAL);

			if(uart_rx_enabled(uart))
				pinMode(3, SPECIAL);

			IOSWAP &= ~(1 << IOSWAPU0);
		}

		break;

	case UART1:
		// Currently no swap possible! See GPIO pins used by UART
		break;

	default:
		break;
	}
}

void uart_set_tx(uart_t* uart, int tx_pin)
{
	if(!uart)
		return;

	switch(uart->uart_nr) {
	case UART0:
		if(uart_tx_enabled(uart)) {
			if(uart->tx_pin == 1 && tx_pin == 2) {
				pinMode(uart->tx_pin, INPUT);
				uart->tx_pin = 2;
				pinMode(uart->tx_pin, FUNCTION_4);
			} else if(uart->tx_pin == 2 && tx_pin != 2) {
				pinMode(uart->tx_pin, INPUT);
				uart->tx_pin = 1;
				pinMode(uart->tx_pin, SPECIAL);
			}
		}

		break;

	case UART1:
		// GPIO7 as TX not possible! See GPIO pins used by UART
		break;

	default:
		break;
	}
}

void uart_set_pins(uart_t* uart, int tx, int rx)
{
	if(!uart)
		return;

	// Only UART0 allows pin changes
	if(uart->uart_nr == UART0) {
		if(uart_tx_enabled(uart) && uart->tx_pin != tx) {
			if(rx == 13 && tx == 15)
				uart_swap(uart, 15);
			else if(rx == 3 && (tx == 1 || tx == 2)) {
				if(uart->rx_pin != rx)
					uart_swap(uart, tx);
				else
					uart_set_tx(uart, tx);
			}
		}
		if(uart_rx_enabled(uart) && uart->rx_pin != rx && rx == 13 && tx == 15)
			uart_swap(uart, 15);
	}
}


static void uart_debug_putc(char c)
{
	uart_t* uart = uart_get_uart(s_uart_debug_nr);
	if (uart)
		uart_write_char(uart, c);
}

void uart_set_debug(int uart_nr)
{
	uart_t* uart = uart_get_uart(uart_nr);

	if (uart == nullptr) {
		s_uart_debug_nr = UART_NO;
		system_set_os_print(false);
	}
	else {
		s_uart_debug_nr = uart_nr;
		system_set_os_print(true);
	}

	ets_install_putc1(uart_debug_putc);
}

int uart_get_debug()
{
	return s_uart_debug_nr;
}

void IRAM_ATTR uart_detach(int uart_nr)
{
	uart_disable_interrupts();
	isrMask &= ~_BV(uart_nr);
	USC1(uart_nr) = 0;
	USIC(uart_nr) = 0xffff;
	USIE(uart_nr) = 0;
	uart_restore_interrupts();
}

void uart_detach_all()
{
	uart_disable_interrupts();
	for (unsigned uart_nr = 0; uart_nr < UART_COUNT; ++uart_nr) {
		USC1(uart_nr) = 0;
		USIC(uart_nr) = 0xffff;
		USIE(uart_nr) = 0;
	}
	isrMask = 0;
}
