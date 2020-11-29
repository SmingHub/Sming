/*
 uart.h - UART HAL

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

*/

/*
 * @author 21/8/2018 mikee47 <mike@sillyhouse.net>
 *
 * SerialBuffer class added to manage FIFO for both RX and TX
 * Buffering is optional; if size is 0 will just use Hardware FIFOS (128 bytes each)
 * For uart_write_xxx functions, if there is no space in the tx FIFO it will return a short count. It will not wait.
 * Trivial check routines have been moved into this header as static inlines.
 * Code is now C++ only; reference to this header has been removed from esp_systemapi.h uart structure should be
 * treated as opaque and only accessed using the functions defined in this header.
 * Callback is invoked on transmit completion.
 *
 * Note: uart_detach_all() should be called at startup, i.e. from user_init().
 */

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <esp_systemapi.h>

/**
 * @defgroup uart_driver UART Driver
 * @ingroup drivers
 * @{
 */

#define UART_NO -1 ///< No UART specified

#define UART_PIN_DEFAULT (255)  ///< Use default pin assignments
#define UART_PIN_NO_CHANGE (-1) ///< Use default pin assignments

// Options for `config` argument of uart_init
#define UART_NB_BIT_MASK 0B00001100
#define UART_NB_BIT_5 0B00000000
#define UART_NB_BIT_6 0B00000100
#define UART_NB_BIT_7 0B00001000
#define UART_NB_BIT_8 0B00001100

#define UART_PARITY_MASK 0B00000011
#define UART_PARITY_NONE 0B00000000
#define UART_PARITY_EVEN 0B00000010
#define UART_PARITY_ODD 0B00000011

#define UART_NB_STOP_BIT_MASK 0B00110000
#define UART_NB_STOP_BIT_0 0B00000000
#define UART_NB_STOP_BIT_1 0B00010000
#define UART_NB_STOP_BIT_15 0B00100000
#define UART_NB_STOP_BIT_2 0B00110000

#define UART_5N1 (UART_NB_BIT_5 | UART_PARITY_NONE | UART_NB_STOP_BIT_1)
#define UART_6N1 (UART_NB_BIT_6 | UART_PARITY_NONE | UART_NB_STOP_BIT_1)
#define UART_7N1 (UART_NB_BIT_7 | UART_PARITY_NONE | UART_NB_STOP_BIT_1)
#define UART_8N1 (UART_NB_BIT_8 | UART_PARITY_NONE | UART_NB_STOP_BIT_1)
#define UART_5N2 (UART_NB_BIT_5 | UART_PARITY_NONE | UART_NB_STOP_BIT_2)
#define UART_6N2 (UART_NB_BIT_6 | UART_PARITY_NONE | UART_NB_STOP_BIT_2)
#define UART_7N2 (UART_NB_BIT_7 | UART_PARITY_NONE | UART_NB_STOP_BIT_2)
#define UART_8N2 (UART_NB_BIT_8 | UART_PARITY_NONE | UART_NB_STOP_BIT_2)
#define UART_5E1 (UART_NB_BIT_5 | UART_PARITY_EVEN | UART_NB_STOP_BIT_1)
#define UART_6E1 (UART_NB_BIT_6 | UART_PARITY_EVEN | UART_NB_STOP_BIT_1)
#define UART_7E1 (UART_NB_BIT_7 | UART_PARITY_EVEN | UART_NB_STOP_BIT_1)
#define UART_8E1 (UART_NB_BIT_8 | UART_PARITY_EVEN | UART_NB_STOP_BIT_1)
#define UART_5E2 (UART_NB_BIT_5 | UART_PARITY_EVEN | UART_NB_STOP_BIT_2)
#define UART_6E2 (UART_NB_BIT_6 | UART_PARITY_EVEN | UART_NB_STOP_BIT_2)
#define UART_7E2 (UART_NB_BIT_7 | UART_PARITY_EVEN | UART_NB_STOP_BIT_2)
#define UART_8E2 (UART_NB_BIT_8 | UART_PARITY_EVEN | UART_NB_STOP_BIT_2)
#define UART_5O1 (UART_NB_BIT_5 | UART_PARITY_ODD | UART_NB_STOP_BIT_1)
#define UART_6O1 (UART_NB_BIT_6 | UART_PARITY_ODD | UART_NB_STOP_BIT_1)
#define UART_7O1 (UART_NB_BIT_7 | UART_PARITY_ODD | UART_NB_STOP_BIT_1)
#define UART_8O1 (UART_NB_BIT_8 | UART_PARITY_ODD | UART_NB_STOP_BIT_1)
#define UART_5O2 (UART_NB_BIT_5 | UART_PARITY_ODD | UART_NB_STOP_BIT_2)
#define UART_6O2 (UART_NB_BIT_6 | UART_PARITY_ODD | UART_NB_STOP_BIT_2)
#define UART_7O2 (UART_NB_BIT_7 | UART_PARITY_ODD | UART_NB_STOP_BIT_2)
#define UART_8O2 (UART_NB_BIT_8 | UART_PARITY_ODD | UART_NB_STOP_BIT_2)

// Status values
enum smg_uart_status_t {
	UART_STATUS_RXFIFO_TOUT = BIT(8),
	UART_STATUS_BRK_DET = BIT(7),
	UART_STATUS_CTS_CHG = BIT(6),
	UART_STATUS_DSR_CHG = BIT(5),
	UART_STATUS_RXFIFO_OVF = BIT(4),
	UART_STATUS_FRM_ERR = BIT(3),
	UART_STATUS_PARITY_ERR = BIT(2),
	UART_STATUS_TXFIFO_EMPTY = BIT(1),
	UART_STATUS_RXFIFO_FULL = BIT(0),
};

/** @brief values for `mode` argument of uart_init */
enum smg_uart_mode_t {
	UART_FULL,	///< Both receive and transmit - will revert to TX only if RX not supported
	UART_RX_ONLY, ///< Receive only
	UART_TX_ONLY  ///< Transmit only
};

using uart_options_t = uint8_t;

/** @brief bit values for `options` argument of uart_init
 *  @note use _BV(opt) to specify values
 */
enum smg_uart_option_bits_t {
	UART_OPT_TXWAIT,	   ///< If buffers are full then uart_write() will wait for free space
	UART_OPT_CALLBACK_RAW, ///< ISR invokes user callback function with no pre-processing
};

#define UART_RX_FIFO_SIZE 0x80
#define UART_TX_FIFO_SIZE 0x80

struct smg_uart_t;

/** @brief callback invoked directly from ISR
 *  @param arg the UART object
 *  @param status UART USIS STATUS flag bits indicating cause of interrupt
 *  @note Values can be:
 *  	UIFE: TX FIFO Empty
 *  	UIFF: RX FIFO Full
 *  	UITO: RX FIFO Timeout
 *  	UIBD: Break Detected
 *
 * Errors can be detected via uart_get_status().
 */
typedef void (*smg_uart_callback_t)(smg_uart_t* uart, uint32_t status);

/*
 * Port notifications
 */

/** @brief Indicates notification, parameters refer to uart_notify_info_t structure
 */
enum smg_uart_notify_code_t {
	/** @brief Called when uart has been initialised successfully */
	UART_NOTIFY_AFTER_OPEN,

	/** @brief Called immediately before uart is closed and destroyed */
	UART_NOTIFY_BEFORE_CLOSE,

	/** @brief Called after data has been written into tx buffer */
	UART_NOTIFY_AFTER_WRITE,

	/** @brief Called before data is read from rx buffer */
	UART_NOTIFY_BEFORE_READ,

	/** @brief Called to ensure all buffered data is output */
	UART_NOTIFY_WAIT_TX,
};

/** @brief Port notification callback function type
 *  @param info
 *  @retval bool true if callback handled operation, false to default to normal operation
 */
using smg_uart_notify_callback_t = void (*)(smg_uart_t* uart, smg_uart_notify_code_t code);

/** @brief Set the notification callback function
 *  @param uart_nr Which uart to register notifications for
 *  @param callback
 *  @retval bool true on success
 */
bool smg_uart_set_notify(unsigned uart_nr, smg_uart_notify_callback_t callback);

struct SerialBuffer;

struct smg_uart_t {
	uint8_t uart_nr;
	uint32_t baud_rate;
	smg_uart_mode_t mode;
	uint8_t options;
	uint8_t rx_pin;
	uint8_t tx_pin;
	uint8_t rx_headroom;			///< Callback when rx_buffer free space <= headroom
	uint16_t status;				///< All status flags reported to callback since last uart_get_status() call
	struct SerialBuffer* rx_buffer; ///< Optional receive buffer
	struct SerialBuffer* tx_buffer; ///< Optional transmit buffer
	smg_uart_callback_t callback;   ///< Optional User callback routine
	void* param;					///< User-supplied callback parameter
};

struct smg_uart_config_t {
	uint8_t uart_nr;
	uint8_t tx_pin; ///< Specify 2 for alternate pin, otherwise defaults to pin 1
	uint8_t rx_pin;
	smg_uart_mode_t mode; ///< Whether to enable receive, transmit or both
	uart_options_t options;
	uint32_t baudrate; ///< Requested baudrate; actual baudrate may differ
	uint32_t config;   ///< UART CONF0 register bits
	size_t rx_size;
	size_t tx_size;
};

// @deprecated Use `smg_uart_init_ex()` instead
smg_uart_t* smg_uart_init(uint8_t uart_nr, uint32_t baudrate, uint32_t config, smg_uart_mode_t mode, uint8_t tx_pin,
						  size_t rx_size, size_t tx_size = 0);

smg_uart_t* smg_uart_init_ex(const smg_uart_config_t& cfg);

void smg_uart_uninit(smg_uart_t* uart);

__forceinline int smg_uart_get_nr(smg_uart_t* uart)
{
	return uart ? uart->uart_nr : -1;
}

/** @brief Get the uart structure for the given number
 *  @param uart_nr
 *  @retval uart_t* Returns nullptr if uart isn't initialised
 */
smg_uart_t* smg_uart_get_uart(uint8_t uart_nr);

/** @brief Set callback handler for serial port
 *  @param uart
 *  @param callback specify nullptr to disable callbacks
 *  @param param user parameter passed to callback
 */
void smg_uart_set_callback(smg_uart_t* uart, smg_uart_callback_t callback, void* param);

/** @brief Get the callback parameter specified by uart_set_callback()
 *  @param uart
 *  @retval void* the callback parameter
 */
__forceinline void* smg_uart_get_callback_param(smg_uart_t* uart)
{
	return uart ? uart->param : nullptr;
}

/** @brief Set option flags
 *  @param uart
 *  @param options The option(s) to set
 */
static inline void smg_uart_set_options(smg_uart_t* uart, uart_options_t options)
{
	if(uart)
		uart->options = options;
}

/** @brief Get error flags and clear them
 *  @param uart
 *  @retval Status error bits:
 *  @note To detect errors during a transaction, call at the start to clear the flags,
 *  then check the value at the end.
 *	Only these values are cleared/returned:
 *  	UIBD: Break Detected
 *  	UIOF: RX FIFO OverFlow
 *  	UIFR: Frame Error
 *  	UIPE: Parity Error
 */
uint8_t IRAM_ATTR smg_uart_get_status(smg_uart_t* uart);

static inline uart_options_t smg_uart_get_options(smg_uart_t* uart)
{
	return uart ? uart->options : 0;
}

void smg_uart_swap(smg_uart_t* uart, int tx_pin);
bool smg_uart_set_tx(smg_uart_t* uart, int tx_pin);
bool smg_uart_set_pins(smg_uart_t* uart, int tx_pin, int rx_pin);

__forceinline bool smg_uart_tx_enabled(smg_uart_t* uart)
{
	return uart && uart->mode != UART_RX_ONLY;
}

__forceinline bool smg_uart_rx_enabled(smg_uart_t* uart)
{
	return uart && uart->mode != UART_TX_ONLY;
}

/** @brief set UART baud rate, given the UART number
 *  @param uart_nr
 *  @param baud_rate requested baud rate
 *  @retval uint32_t actual baudrate used, 0 on failure
 */
uint32_t smg_uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate);

/** @brief set UART baud rate
 *  @param uart
 *  @param baud_rate requested baud rate
 *  @retval uint32_t actual baudrate used, 0 on failure
 */
uint32_t smg_uart_set_baudrate(smg_uart_t* uart, uint32_t baud_rate);

/** @brief get the actual baud rate in use
 *  @param uart
 *  @retval uint32_t the baud rate, 0 on failure
 */
uint32_t smg_uart_get_baudrate(smg_uart_t* uart);

size_t smg_uart_resize_rx_buffer(smg_uart_t* uart, size_t new_size);
size_t smg_uart_rx_buffer_size(smg_uart_t* uart);
size_t smg_uart_resize_tx_buffer(smg_uart_t* uart, size_t new_size);
size_t smg_uart_tx_buffer_size(smg_uart_t* uart);

/** @brief write a block of data
 *  @param uart
 *  @param buffer
 *  @param size
 *  @retval size_t number of bytes buffered for transmission
 */
size_t smg_uart_write(smg_uart_t* uart, const void* buffer, size_t size);

/** @brief queue a single character for output
 *  @param uart
 *  @param c
 *  @retval size_t 1 if character was written, 0 on failure
 */
static inline size_t smg_uart_write_char(smg_uart_t* uart, char c)
{
	return smg_uart_write(uart, &c, 1);
}

/** @brief read a block of data
 *  @param uart
 *  @param buffer where to write the data
 *  @param size requested quantity of bytes to read
 *  @retval size_t number of bytes read
 */
size_t smg_uart_read(smg_uart_t* uart, void* buffer, size_t size);

/** @brief read a received character
 *  @param uart
 *  @retval the character, -1 on failure
 */
static inline int smg_uart_read_char(smg_uart_t* uart)
{
	char c;
	return smg_uart_read(uart, &c, 1) ? c : -1;
}

/** @brief see what the next character in the rx buffer is
 *  @param uart
 *  @retval int returns -1 if buffer is empty or not allocated
 *  @note if buffer isn't allocated data may be in the hardware FIFO, which
 *  must be read out using uart_read()
 */
int smg_uart_peek_char(smg_uart_t* uart);

/** @brief fetch last character read out of FIFO
 *  @param uart
 *  @retval int the character, or -1 if rx buffer is empty or unallocated
 *  @note this is only useful if an rx buffer has been allocated of sufficient size
 *  to contain a message. This function then indicates the terminating character.
 */
int smg_uart_peek_last_char(smg_uart_t* uart);

/*
 * @brief Find a character in the receive buffer
 * @param uart
 * @param char c character to search for
 * @retval size_t position relative to start of buffer, -1 if not found
 */
int smg_uart_rx_find(smg_uart_t* uart, char c);

/** @brief determine available data which can be read
 *  @param uart
 *  @retval size_t
 *  @note this obtains a count of data both in the memory buffer and hardware FIFO
 */
size_t smg_uart_rx_available(smg_uart_t* uart);

/** @brief return free space in transmit buffer */
size_t smg_uart_tx_free(smg_uart_t* uart);

/** @deprecated don't use this - causes extended delays - use callback notification */
void smg_uart_wait_tx_empty(smg_uart_t* uart);

/** @brief Set or clear a break condition on the TX line
 *  @param uart
 *  @param state
 */
void smg_uart_set_break(smg_uart_t* uart, bool state);

/** @brief discard any buffered data and reset hardware FIFOs
 *  @param uart
 *  @param mode Whether to flush TX, RX or both (the default)
 *  @note this function does not wait for any transmissions to complete
 */
void smg_uart_flush(smg_uart_t* uart, smg_uart_mode_t mode = UART_FULL);

void smg_uart_set_debug(int uart_nr);
int smg_uart_get_debug();

/** @brief enable interrupts for a UART
 *  @param uart
 */
void smg_uart_start_isr(smg_uart_t* uart);

/** @brief disable interrupts for a UART
 *  @param uart
 */
void __forceinline smg_uart_stop_isr(smg_uart_t* uart)
{
	extern void smg_uart_detach(int);
	if(uart != nullptr) {
		smg_uart_detach(uart->uart_nr);
	}
}

/** @brief detach a UART interrupt service routine
 *  @param uart_nr
 */
void smg_uart_detach(int uart_nr);

/** @brief detach all UART interrupt service routines
 *  @note call at startup to put all UARTs into a known state
 */
void smg_uart_detach_all();

/** @brief disable interrupts and return current interrupt state
 *  @retval state non-zero if any UART interrupts were active
 */
uint8_t smg_uart_disable_interrupts();

/** @brief re-enable interrupts after calling uart_disable_interrupts()
 */
void smg_uart_restore_interrupts();

/** @} */

#if defined(__cplusplus)
} // extern "C"
#endif
