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

#include "driver/uart.h"
#include "espinc/peri.h"

#include "SerialBuffer.h"

/*
 * Parameters relating to RX FIFO and buffer thresholds
 *
 * 'headroom' is the number of characters which may be received before a receive overrun
 * condition occurs and data is lost.
 *
 * For the hardware FIFO, data is processed via interrupt so the headroom can be fairly small.
 * The greater the headroom, the more interrupts will be generated thus reducing efficiency.
 */
#define RX_FIFO_FULL_THRESHOLD 120 ///< UIFF interrupt when FIFO bytes > threshold
#define RX_FIFO_HEADROOM (UART_RX_FIFO_SIZE - RX_FIFO_FULL_THRESHOLD) ///< Chars between UIFF and UIOF
/*
 * Using a buffer, data is typically processed via task callback so requires additional time.
 * This figure is set to a nominal default which should provide robust operation for most situations.
 * It can be adjusted if necessary via the rx_headroom parameter.
*/
#define DEFAULT_RX_HEADROOM (32 - RX_FIFO_HEADROOM)

static int s_uart_debug_nr = UART_NO;

// Get number of characters in receive FIFO
__forceinline static uint8_t uart_rxfifo_count(uint8_t nr)
{
	return (USS(nr) >> USRXC) & 0xff;
}

// Get number of characters in transmit FIFO
__forceinline static uint8_t uart_txfifo_count(uint8_t nr)
{
	return (USS(nr) >> USTXC) & 0xff;
}

// Get available free characters in transmit FIFO
__forceinline static uint8_t uart_txfifo_free(uint8_t nr)
{
	return UART_TX_FIFO_SIZE - uart_txfifo_count(nr) - 1;
}

// Return true if transmit FIFO is full
__forceinline static bool uart_txfifo_full(uint8_t nr)
{
	return uart_txfifo_count(nr) >= (UART_TX_FIFO_SIZE - 1);
}

// Keep track of interrupt enable state for each UART
static uint8_t isrMask;
// Keep a reference to all created UARTS - required because they share an ISR
static uart_t* uartInstances[UART_COUNT];

// Registered port callback functions
static uart_notify_callback_t notifyCallbacks[UART_COUNT];

/** @brief Invoke a port callback, if one has been registered
 *  @param uart
 *  @param code
 */
static void notify(uart_t* uart, uart_notify_code_t code)
{
	auto callback = notifyCallbacks[uart->uart_nr];
	if(callback != nullptr) {
		callback(uart, code);
	}
}

__forceinline static bool uart_isr_enabled(uint8_t nr)
{
	return bitRead(isrMask, nr);
}

uart_t* uart_get_uart(uint8_t uart_nr)
{
	return (uart_nr < UART_COUNT) ? uartInstances[uart_nr] : nullptr;
}

uint8_t uart_disable_interrupts()
{
	ETS_UART_INTR_DISABLE();
	return isrMask;
}

void uart_restore_interrupts()
{
	if(isrMask != 0) {
		ETS_UART_INTR_ENABLE();
	}
}

bool uart_set_notify(unsigned uart_nr, uart_notify_callback_t callback)
{
	if(uart_nr >= UART_COUNT) {
		return false;
	}

	notifyCallbacks[uart_nr] = callback;
	return true;
}

/** @brief Determine if the given uart is a real uart or a virtual one
 */
static __forceinline bool is_physical(int uart_nr)
{
	return (uart_nr >= 0) && (uart_nr < UART_PHYSICAL_COUNT);
}

static __forceinline bool is_physical(uart_t* uart)
{
	return uart != nullptr && is_physical(uart->uart_nr);
}

/** @brief If given a virtual uart, obtain the related physical one
 */
static uart_t* get_physical(uart_t* uart)
{
	if(uart != nullptr && uart->uart_nr == UART2) {
		uart = uartInstances[UART0];
	}
	return uart;
}

void uart_set_callback(uart_t* uart, uart_callback_t callback, void* param)
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
			uart_disable_interrupts();
			delete buffer;
			buffer = nullptr;
			uart_restore_interrupts();
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

size_t uart_resize_rx_buffer(uart_t* uart, size_t new_size)
{
	if(uart_rx_enabled(uart)) {
		realloc_buffer(uart->rx_buffer, new_size);
	}
	return uart_rx_buffer_size(uart);
}

size_t uart_rx_buffer_size(uart_t* uart)
{
	return uart != nullptr && uart->rx_buffer != nullptr ? uart->rx_buffer->getSize() : 0;
}

size_t uart_resize_tx_buffer(uart_t* uart, size_t new_size)
{
	if(uart_tx_enabled(uart)) {
		realloc_buffer(uart->tx_buffer, new_size);
	}
	return uart_tx_buffer_size(uart);
}

size_t uart_tx_buffer_size(uart_t* uart)
{
	return uart != nullptr && uart->tx_buffer != nullptr ? uart->tx_buffer->getSize() : 0;
}

int uart_peek_char(uart_t* uart)
{
	return uart != nullptr && uart->rx_buffer ? uart->rx_buffer->peekChar() : -1;
}

int uart_rx_find(uart_t* uart, char c)
{
	if(uart == nullptr || uart->rx_buffer == nullptr) {
		return -1;
	}

	return uart->rx_buffer->find(c);
}

int uart_peek_last_char(uart_t* uart)
{
	return uart != nullptr && uart->rx_buffer != nullptr ? uart->rx_buffer->peekLastChar() : -1;
}

size_t uart_read(uart_t* uart, void* buffer, size_t size)
{
	if(!uart_rx_enabled(uart) || buffer == nullptr || size == 0) {
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

	// Top up from hardware FIFO
	if(is_physical(uart)) {
		while(read < size && uart_rxfifo_count(uart->uart_nr) != 0) {
			buf[read++] = USF(uart->uart_nr);
		}

		// FIFO full may have been disabled if buffer overflowed, re-enabled it now
		USIC(uart->uart_nr) = _BV(UIFF) | _BV(UITO) | _BV(UIOF);
		USIE(uart->uart_nr) |= _BV(UIFF) | _BV(UITO) | _BV(UIOF);
	}

	return read;
}

size_t uart_rx_available(uart_t* uart)
{
	if(!uart_rx_enabled(uart)) {
		return 0;
	}

	uart_disable_interrupts();

	size_t avail = is_physical(uart) ? uart_rxfifo_count(uart->uart_nr) : 0;

	if(uart->rx_buffer != nullptr) {
		avail += uart->rx_buffer->available();
	}

	uart_restore_interrupts();

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
	if(usis == 0) {
		return;
	}

	/*
	 * If we haven't asked for interrupts on this UART, then disable all interrupt sources for it.
	 *
	 * This happens at startup where we've only initialised one of the UARTS. For example, we initialise
	 * UART1 for debug output but leave UART0 alone. However, the SDK has enabled some interrupt sources
	 * which we're not expecting.
	 *
	 * (Calling uart_detach_all() at startup pre-empts all this.)
	 */
	if(uart == nullptr || !uart_isr_enabled(uart_nr)) {
		USIE(uart_nr) = 0;
		return;
	}

	// Value to be passed to callback
	uint32_t status = usis;

	// Deal with the event, unless we're in raw mode
	if(!bitRead(uart->options, UART_OPT_CALLBACK_RAW)) {
		// Rx FIFO full or timeout
		if(usis & (_BV(UIFF) | _BV(UITO) | _BV(UIOF))) {
			size_t read = 0;

			// Read as much data as possible from the RX FIFO into buffer
			if(uart->rx_buffer != nullptr) {
				size_t avail = uart_rxfifo_count(uart_nr);
				size_t space = uart->rx_buffer->getFreeSpace();
				read = (avail <= space) ? avail : space;
				space -= read;
				while(read-- != 0) {
					uart->rx_buffer->writeChar(USF(uart_nr));
				}

				// Don't call back until buffer is (almost) full
				if(space > uart->rx_headroom) {
					bitClear(status, UIFF);
				}
			}

			/*
			 * If the FIFO is full and we didn't read any of the data then need to mask the interrupt out or it'll recur.
			 * The interrupt gets re-enabled by a call to uart_read() or uart_flush()
			 */
			if(bitRead(usis, UIOF)) {
				bitClear(USIE(uart_nr), UIOF);
			} else if(read == 0) {
				USIE(uart_nr) &= ~(_BV(UIFF) | _BV(UITO));
			}
		}

		// Unless we replenish TX FIFO, disable after handling interrupt
		if(bitRead(usis, UIFE)) {
			// Dump as much data as we can from buffer into the TX FIFO
			if(uart->tx_buffer != nullptr) {
				size_t space = uart_txfifo_free(uart_nr);
				size_t avail = uart->tx_buffer->available();
				size_t count = (avail <= space) ? avail : space;
				while(count-- != 0) {
					USF(uart_nr) = uart->tx_buffer->readChar();
				}
			}

			// If TX FIFO remains empty then we must disable TX FIFO EMPTY interrupt to stop it recurring.
			if(uart_txfifo_count(uart_nr) == 0) {
				// The interrupt gets re-enabled by uart_write()
				bitClear(USIE(uart_nr), UIFE);
			} else {
				// We've topped up TX FIFO so defer callback until next time
				bitClear(status, UIFE);
			}
		}
	}

	// Keep a note of persistent flags - cleared via uart_get_status()
	uart->status |= status;

	if(status != 0 && uart->callback != nullptr) {
		uart->callback(uart, status);
	}

	// Final step is to clear status flags
	USIC(uart_nr) = usis;
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
	if(!is_physical(uart)) {
		return;
	}

	uint32_t usc1 = 0;
	uint32_t usie = 0;

	if(uart_rx_enabled(uart)) {
		/* UCFFT: RX FIFO Full Threshold
		 * UCTOT: RX TimeOut Treshold
		 * UCTOE: RX TimeOut Enable
		 */
		usc1 = (120 << UCFFT) | (0x02 << UCTOT) | _BV(UCTOE);
		/*
		 * UIBD: Break Detected
		 * UIOF: RX FIFO OverFlow
		 * UIFF: RX FIFO Full
		 * UIFR: Frame Error
		 * UIPE: Parity Error
		 * UITO: RX FIFO Timeout
		 *
		 * There is little benefit in generating interrupts on errors, instead these
		 * should be cleared at the start of a transaction and checked at the end.
		 * See uart_get_status().
		 */
		usie = _BV(UIFF) | _BV(UITO) | _BV(UIBD) | _BV(UIOF);
	}

	if(uart_tx_enabled(uart)) {
		/*
		 * We can interrupt when TX FIFO is empty; at 1Mbit that gives us 800 CPU
		 * cycles before the last character has actually gone over the wire. Even if
		 * a gap occurs it is unlike to cause any problems. It also makes the callback
		 * more useful, for example if using it for RS485 we'd then want to reverse
		 * transfer direction and begin waiting for a response.
		 */

		// TX FIFO empty threshold
		// usc1 |= (0 << UCFET);
		// TX FIFO empty interrupt only gets enabled via uart_write function()
	}

	USC1(uart->uart_nr) = usc1;
	USIC(uart->uart_nr) = 0xffff;
	USIE(uart->uart_nr) = usie;

	uint8_t oldmask = isrMask;

	bitSet(isrMask, uart->uart_nr);

	if(oldmask == 0) {
		ETS_UART_INTR_DISABLE();
		ETS_UART_INTR_ATTACH(uart_isr, nullptr);
		ETS_UART_INTR_ENABLE();
	}
}

size_t uart_write(uart_t* uart, const void* buffer, size_t size)
{
	if(!uart_tx_enabled(uart) || buffer == nullptr || size == 0) {
		return 0;
	}

	size_t written = 0;

	auto buf = static_cast<const uint8_t*>(buffer);

	bool isPhysical = is_physical(uart);

	while(written < size) {
		if(isPhysical) {
			// If TX buffer not in use or it's empty then write directly to hardware FIFO
			if(uart->tx_buffer == nullptr || uart->tx_buffer->isEmpty()) {
				while(written < size && !uart_txfifo_full(uart->uart_nr)) {
					USF(uart->uart_nr) = buf[written++];
				}
				// Enable TX FIFO EMPTY interrupt
				USIC(uart->uart_nr) = _BV(UIFE);
				bitSet(USIE(uart->uart_nr), UIFE);
			}
		}

		// Write any remaining data into transmit buffer
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

size_t uart_tx_free(uart_t* uart)
{
	if(!uart_tx_enabled(uart)) {
		return 0;
	}

	uart_disable_interrupts();

	size_t space = is_physical(uart) ? uart_txfifo_free(uart->uart_nr) : 0;
	if(uart->tx_buffer != nullptr) {
		space += uart->tx_buffer->getFreeSpace();
	}

	uart_restore_interrupts();

	return space;
}

void uart_wait_tx_empty(uart_t* uart)
{
	if(!uart_tx_enabled(uart)) {
		return;
	}

	notify(uart, UART_NOTIFY_WAIT_TX);

	if(uart->tx_buffer != nullptr) {
		while(!uart->tx_buffer->isEmpty()) {
			delay(0);
		}
	}

	if(is_physical(uart)) {
		while(uart_txfifo_count(uart->uart_nr) != 0)
			delay(0);
	}
}

void uart_set_break(uart_t* uart, bool state)
{
	uart = get_physical(uart);
	if(uart != nullptr) {
		bitWrite(USC0(uart->uart_nr), UCBRK, state);
	}
}

uint8_t uart_get_status(uart_t* uart)
{
	uint8_t status = 0;
	if(uart != nullptr) {
		uart_disable_interrupts();
		// Get break/overflow flags from actual uart (physical or otherwise)
		status = uart->status & (_BV(UIBD) | _BV(UIOF));
		uart->status = 0;
		// Read raw status register directly from real uart, masking out non-error bits
		uart = get_physical(uart);
		if(uart != nullptr) {
			status |= USIR(uart->uart_nr) & (_BV(UIBD) | _BV(UIOF) | _BV(UIFR) | _BV(UIPE));
			// Clear errors
			USIC(uart->uart_nr) = status;
		}
		uart_restore_interrupts();
	}
	return status;
}

void uart_flush(uart_t* uart, uart_mode_t mode)
{
	if(uart == nullptr) {
		return;
	}

	bool flushRx = mode != UART_TX_ONLY && uart->mode != UART_TX_ONLY;
	bool flushTx = mode != UART_RX_ONLY && uart->mode != UART_RX_ONLY;

	uart_disable_interrupts();
	if(flushRx && uart->rx_buffer != nullptr) {
		uart->rx_buffer->clear();
	}

	if(flushTx && uart->tx_buffer != nullptr) {
		uart->tx_buffer->clear();
	}

	if(is_physical(uart)) {
		// Clear the hardware FIFOs
		uint32_t flushBits = 0;
		if(flushTx) {
			bitSet(flushBits, UCTXRST);
		}
		if(flushRx) {
			bitSet(flushBits, UCRXRST);
		}
		USC0(uart->uart_nr) |= flushBits;
		USC0(uart->uart_nr) &= ~flushBits;

		if(flushTx) {
			// Prevent TX FIFO EMPTY interrupts - don't need them until uart_write is called again
			bitClear(USIE(uart->uart_nr), UIFE);
		}

		// If receive overflow occurred then these interrupts will be masked
		if(flushRx) {
			USIC(uart->uart_nr) = 0xffff & ~_BV(UIFE);
			USIE(uart->uart_nr) |= _BV(UIFF) | _BV(UITO) | _BV(UIOF);
		}
	}

	uart_restore_interrupts();
}

uint32_t uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate)
{
	if(!is_physical(uart_nr) || baud_rate == 0) {
		return 0;
	}

	uint32_t clkdiv = ESP8266_CLOCK / baud_rate;
	USD(uart_nr) = clkdiv;
	// Return the actual baud rate in use
	baud_rate = clkdiv ? ESP8266_CLOCK / clkdiv : 0;
	return baud_rate;
}

uint32_t uart_set_baudrate(uart_t* uart, uint32_t baud_rate)
{
	uart = get_physical(uart);
	if(uart == nullptr) {
		return 0;
	}

	baud_rate = uart_set_baudrate_reg(uart->uart_nr, baud_rate);
	// Store the actual baud rate in use
	uart->baud_rate = baud_rate;
	return baud_rate;
}

uint32_t uart_get_baudrate(uart_t* uart)
{
	uart = get_physical(uart);
	return (uart == nullptr) ? 0 : uart->baud_rate;
}

uart_t* uart_init_ex(const uart_config& cfg)
{
	// Already initialised?
	if(uart_get_uart(cfg.uart_nr) != nullptr) {
		return nullptr;
	}

	auto uart = new uart_t;
	if(uart == nullptr) {
		return nullptr;
	}

	memset(uart, 0, sizeof(uart_t));
	uart->uart_nr = cfg.uart_nr;
	uart->mode = cfg.mode;
	uart->options = cfg.options;
	uart->tx_pin = 255;
	uart->rx_pin = 255;
	uart->rx_headroom = DEFAULT_RX_HEADROOM;

	auto rxBufferSize = cfg.rx_size;
	auto txBufferSize = cfg.tx_size;

	switch(cfg.uart_nr) {
	case UART0:
	case UART2:
		// Virtual uart requires a minimum RAM buffer
		if(cfg.uart_nr == UART2) {
			rxBufferSize += UART_RX_FIFO_SIZE;
			txBufferSize += UART_TX_FIFO_SIZE;
		}

		if(uart_rx_enabled(uart) && !realloc_buffer(uart->rx_buffer, rxBufferSize)) {
			delete uart;
			return nullptr;
		}

		if(uart_tx_enabled(uart) && !realloc_buffer(uart->tx_buffer, txBufferSize)) {
			delete uart->rx_buffer;
			delete uart;
			return nullptr;
		}

		if(cfg.uart_nr == UART2) {
			break;
		}

		// OK, buffers allocated so setup hardware
		uart_detach(cfg.uart_nr);

		if(uart_rx_enabled(uart)) {
			uart->rx_pin = 3;
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
		}

		bitClear(IOSWAP, IOSWAPU0);
		USC0(UART0) = cfg.config;
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
		uart_detach(cfg.uart_nr);
		// GPIO7 as TX not possible! See GPIO pins used by UART
		uart->tx_pin = 2;
		pinMode(uart->tx_pin, SPECIAL);
		USC0(UART1) = cfg.config;
		break;

	default:
		// big fail!
		delete uart;
		return nullptr;
	}

	uart_set_baudrate(uart, cfg.baudrate);
	uart_flush(uart);
	uartInstances[cfg.uart_nr] = uart;
	uart_start_isr(uart);

	notify(uart, UART_NOTIFY_AFTER_OPEN);

	return uart;
}

void uart_uninit(uart_t* uart)
{
	if(uart == nullptr) {
		return;
	}

	notify(uart, UART_NOTIFY_BEFORE_CLOSE);

	uart_stop_isr(uart);
	// If debug output being sent to this UART, disable it
	if(uart->uart_nr == s_uart_debug_nr) {
		uart_set_debug(UART_NO);
	}

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

uart_t* uart_init(uint8_t uart_nr, uint32_t baudrate, uint32_t config, uart_mode_t mode, uint8_t tx_pin, size_t rx_size,
				  size_t tx_size)
{
	uart_config cfg = {.uart_nr = uart_nr,
					   .tx_pin = tx_pin,
					   .mode = mode,
					   .options = _BV(UART_OPT_TXWAIT),
					   .baudrate = baudrate,
					   .config = config,
					   .rx_size = rx_size,
					   .tx_size = tx_size};
	return uart_init_ex(cfg);
}

void uart_swap(uart_t* uart, int tx_pin)
{
	if(uart == nullptr) {
		return;
	}

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

			bitSet(IOSWAP, IOSWAPU0);
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

			bitClear(IOSWAP, IOSWAPU0);
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
	if(uart == nullptr) {
		return;
	}

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
	if(uart == nullptr) {
		return;
	}

	// Only UART0 allows pin changes
	if(uart->uart_nr == UART0) {
		if(uart_tx_enabled(uart) && uart->tx_pin != tx) {
			if(rx == 13 && tx == 15)
				uart_swap(uart, 15);
			else if(rx == 3 && (tx == 1 || tx == 2)) {
				if(uart->rx_pin != rx) {
					uart_swap(uart, tx);
				} else {
					uart_set_tx(uart, tx);
				}
			}
		}
		if(uart_rx_enabled(uart) && uart->rx_pin != rx && rx == 13 && tx == 15) {
			uart_swap(uart, 15);
		}
	}
}

void uart_debug_putc(char c)
{
	uart_t* uart = uart_get_uart(s_uart_debug_nr);
	if(uart != nullptr) {
		uart_write_char(uart, c);
	}
}

void uart_set_debug(int uart_nr)
{
	s_uart_debug_nr = uart_nr;
	system_set_os_print(uart_nr >= 0);
	ets_install_putc1(uart_debug_putc);
}

int uart_get_debug()
{
	return s_uart_debug_nr;
}

void uart_detach(int uart_nr)
{
	if(!is_physical(uart_nr)) {
		return;
	}

	uart_disable_interrupts();
	bitClear(isrMask, uart_nr);
	USC1(uart_nr) = 0;
	USIC(uart_nr) = 0xffff;
	USIE(uart_nr) = 0;
	uart_restore_interrupts();
}

void uart_detach_all()
{
	uart_disable_interrupts();
	for(unsigned uart_nr = 0; uart_nr < UART_PHYSICAL_COUNT; ++uart_nr) {
		USC1(uart_nr) = 0;
		USIC(uart_nr) = 0xffff;
		USIE(uart_nr) = 0;
	}
	isrMask = 0;
}
