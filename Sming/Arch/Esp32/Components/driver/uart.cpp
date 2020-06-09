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
#include <BitManipulations.h>

#include "include/driver/uart.h"
#include "espinc/uart_register.h"
#include "espinc/pin_mux_register.h"
#include <esp32/rom/ets_sys.h>
#include <soc/dport_access.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "SerialBuffer.h"

#include "esp_log.h"

static const char *TAG = "uart_events";

static QueueHandle_t uartQueues[UART_COUNT];
static bool doneHandler[UART_COUNT] = {};

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

smg_uart_t* smg_uart_get_uart(uint8_t uart_nr)
{
	return (uart_nr < UART_COUNT) ? uartInstances[uart_nr] : nullptr;
}

bool smg_uart_set_notify(unsigned uart_nr, smg_uart_notify_callback_t callback)
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

static __forceinline bool is_physical(smg_uart_t* uart)
{
	return uart != nullptr && is_physical(uart->uart_nr);
}

/** @brief If given a virtual uart, obtain the related physical one
 */
static smg_uart_t* get_physical(smg_uart_t* uart)
{
	if(uart != nullptr && uart->uart_nr == UART2) {
		uart = uartInstances[UART0];
	}
	return uart;
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
			smg_uart_disable_interrupts();
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

	// Top up from hardware FIFO
	if(is_physical(uart)) {
		if(read < size) {
			int received = uart_read_bytes(uart->uart_nr, &buf[read], (size - read), 0);
			if(received > 0) {
				read += received;
			}
		}
	}

	return read;
}

size_t smg_uart_rx_available(smg_uart_t* uart)
{
	if(!smg_uart_rx_enabled(uart)) {
		return 0;
	}

	smg_uart_disable_interrupts();

	size_t avail = 0;

	if(uart->rx_buffer != nullptr) {
		avail += uart->rx_buffer->available();
	}

	smg_uart_restore_interrupts();

	return avail;
}

static void smg_uart_event_handler(uart_port_t uart_nr)
{
    uart_event_t event;
    auto queue = uartQueues[uart_nr];

    auto uart = smg_uart_get_uart(uart_nr);
    if(uart == nullptr) {
    	vTaskDelete(NULL);
    	return;
    }

    uint32_t status = 0;
    while(!doneHandler[uart_nr]) {
    	if(xQueueReceive(uartQueues[uart_nr], (void * )&event, (portTickType)portMAX_DELAY)) {
//    			debugf("event type: %d", event.type);

    			switch(event.type) {
    				case UART_FIFO_OVF:
    					uart_get_buffered_data_len(uart_nr, &event.size);
    			    	status = UART_RXFIFO_OVF_INT_ST; //Event of HW FIFO overflow detected
    			    	/* fall-through */
    			    case UART_BUFFER_FULL:
    			    	if(!status) {
    			    		uart_get_buffered_data_len(uart_nr, &event.size);
    			    		status = UART_RXFIFO_FULL_INT_ST; //Event of UART ring buffer full
    			    	}
    			    	/* fall-through */
    				case UART_DATA:
    					if(!status) {
    						status = UART_RXFIFO_TOUT_INT_ST; //Event of UART receiving data
    					}

    					if(uart->rx_buffer != nullptr) {

    						size_t avail = event.size;

    						size_t space = uart->rx_buffer->getFreeSpace();
    						size_t read = (avail <= space) ? avail : space;
    						space -= read;
    						while(read-- != 0) {
    							uint8_t c;
    							int ret = uart_read_bytes(uart_nr, &c, 1, 0);
    							if(ret == -1) {
    								break; //
    							}

    							if(!uart->rx_buffer->writeChar(c)) {
    								break;
    							}
    						}

    						// Don't call back until buffer is (almost) full
    						if(space > uart->rx_headroom) {
    							status &= ~UART_RXFIFO_FULL_INT_ST;
    						}

//    						debugf("avail: %d, read: %d", avail, read);
    					}
    					break;
    				//Event of UART RX break detected
    				case UART_BREAK:
    					break;
    				//Event of UART parity check error
    				case UART_PARITY_ERR:
    					break;
    				//Event of UART frame error
    				case UART_FRAME_ERR:
    					break;
    				//UART_PATTERN_DET
    				case UART_PATTERN_DET:
    					break;
    	//			case UART_DATA_BREAK:
    	//				// Dump as much data as we can from buffer into the TX FIFO
    	//				if(uart->tx_buffer != nullptr) {
    	//					size_t space = smg_uart_txfifo_free(uart_nr);
    	//					size_t avail = uart->tx_buffer->available();
    	//					size_t count = (avail <= space) ? avail : space;
    	//					while(count-- != 0) {
    	//						WRITE_PERI_REG(UART_FIFO(uart_nr), uart->tx_buffer->readChar());
    	//					}
    	//				}
    	//				break;
    				//Others
    				default:
    					break;
    		} // end switch

			uart->status |= status;

			if(status != 0 && uart->callback != nullptr) {
//				debugf("callback with status: %d", status);
				uart->callback(uart, status);
			}

	    	status = 0;
    	} // end if
	} // end while

    vTaskDelete(NULL);
}

void smg_uart_event_task(void *pvParameters)
{
	uint8_t* port =  reinterpret_cast<uint8_t*>(pvParameters);
	smg_uart_event_handler((uart_port_t)(*port));
}

size_t smg_uart_write(smg_uart_t* uart, const void* buffer, size_t size)
{
	if(!smg_uart_tx_enabled(uart) || buffer == nullptr || size == 0) {
		return 0;
	}

	size_t written = 0;

	auto buf = static_cast<const uint8_t*>(buffer);

	bool isPhysical = is_physical(uart);

	while(written < size) {
		if(isPhysical) {
			// If TX buffer not in use or it's empty then write directly to hardware FIFO
			if(uart->tx_buffer == nullptr || uart->tx_buffer->isEmpty()) {
				int sent = uart_write_bytes(uart->uart_nr, (const char* )&buf[written], (size - written));
				if(sent > 0) {
					written += sent;
				}
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

uint8_t smg_uart_get_status(smg_uart_t* uart)
{
	uint8_t status = 0;
	if(uart != nullptr) {
		// TODO:
//		smg_uart_disable_interrupts();
//		// Get break/overflow flags from actual uart (physical or otherwise)
//		status = uart->status & (UART_BRK_DET_INT_ST | UART_RXFIFO_OVF_INT_ST);
//		uart->status = 0;
//		// Read raw status register directly from real uart, masking out non-error bits
//		uart = get_physical(uart);
//		if(uart != nullptr) {
//			uint32_t intraw = READ_PERI_REG(UART_INT_RAW(uart->uart_nr));
//			intraw &= UART_BRK_DET_INT_ST | UART_RXFIFO_OVF_INT_ST | UART_FRM_ERR_INT_ST | UART_PARITY_ERR_INT_ST;
//			status |= intraw;
//			// Clear errors
//			WRITE_PERI_REG(UART_INT_CLR(uart->uart_nr), status);
//		}
//		smg_uart_restore_interrupts();
	}
	return status;
}

void smg_uart_flush(smg_uart_t* uart, smg_uart_mode_t mode)
{
	// TODO: check if mode can be set.
	uart_flush(uart->uart_nr);
}

void smg_uart_wait_tx_empty(smg_uart_t* uart)
{
	uart = get_physical(uart);
	if(uart == nullptr) {
		return;
	}

	uart_wait_tx_done(uart->uart_nr, portMAX_DELAY);
}

uint32_t smg_uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate)
{
	if(!is_physical(uart_nr) || baud_rate == 0) {
		return 0;
	}

	uart_set_baudrate((uart_port_t)uart_nr, baud_rate);

	return baud_rate;
}

uint32_t smg_uart_set_baudrate(smg_uart_t* uart, uint32_t baud_rate)
{
	uart = get_physical(uart);
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
	uart = get_physical(uart);
	return (uart == nullptr) ? 0 : uart->baud_rate;
}

smg_uart_t* smg_uart_init_ex(const smg_uart_config& cfg)
{
	//Set UART log level
	esp_log_level_set(TAG, ESP_LOG_INFO);


	// Already initialised?
	if(smg_uart_get_uart(cfg.uart_nr) != nullptr) {
		return nullptr;
	}

	auto uart = new smg_uart_t;
	if(uart == nullptr) {
		return nullptr;
	}

	memset(uart, 0, sizeof(smg_uart_t));
	uart->uart_nr = (uart_port_t)cfg.uart_nr;
	uart->mode = cfg.mode;
	uart->options = cfg.options;
	uart->tx_pin = 255;
	uart->rx_pin = 255;
	uart->rx_headroom = DEFAULT_RX_HEADROOM;

	uart_config_t uart_config = {
	        .baud_rate = (int)cfg.baudrate,
	        .data_bits = UART_DATA_8_BITS,
	        .parity    = UART_PARITY_DISABLE,
	        .stop_bits = UART_STOP_BITS_1,
	        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	if(!realloc_buffer(uart->rx_buffer, cfg.rx_size)) {
		delete uart;
		return nullptr;
	}

	if(!realloc_buffer(uart->tx_buffer, cfg.tx_size)) {
		delete uart;
		return nullptr;
	}

	uart_port_t port = (uart_port_t)cfg.uart_nr;
	uart_param_config(port, &uart_config);
	uart_set_pin(port, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	uart_driver_install(port, cfg.rx_size, cfg.tx_size, 20, &uartQueues[cfg.uart_nr], 0);

	uartInstances[cfg.uart_nr] = uart;

    //Create a task to handle UART event from ISR
	doneHandler[uart->uart_nr] = false;
    xTaskCreate(smg_uart_event_task, "uart_event_task", 2048, &uart->uart_nr, 12, NULL);

	notify(uart, UART_NOTIFY_AFTER_OPEN);

	return uart;
}

void smg_uart_uninit(smg_uart_t* uart)
{
	if(uart == nullptr) {
		return;
	}

	notify(uart, UART_NOTIFY_BEFORE_CLOSE);

	// If debug output being sent to this UART, disable it
	if(uart->uart_nr == s_uart_debug_nr) {
		smg_uart_set_debug(UART_NO);
	}

	doneHandler[uart->uart_nr] = true;
	uart_driver_delete(uart->uart_nr);

	uartInstances[uart->uart_nr] = nullptr;

	delete uart->rx_buffer;
	delete uart->tx_buffer;
	delete uart;
}

smg_uart_t* smg_uart_init(uint8_t uart_nr, uint32_t baudrate, uint32_t config, smg_uart_mode_t mode, uint8_t tx_pin, size_t rx_size,
				  size_t tx_size)
{
	smg_uart_config cfg = {.uart_nr = (uart_port_t)uart_nr,
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
	if(uart == nullptr) {
		return;
	}

	switch(uart->uart_nr) {
	case UART0:
	case UART1:
		// Currently no swap possible! See GPIO pins used by UART
		break;

	default:
		break;
	}
}

void smg_uart_set_tx(smg_uart_t* uart, int tx_pin)
{
	if(uart == nullptr) {
		return;
	}

	// TODO:
}

void smg_uart_set_pins(smg_uart_t* uart, int tx, int rx)
{
	if(uart == nullptr) {
		return;
	}

	uart_set_pin(uart->uart_nr, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
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
//	s_uart_debug_nr = uart_nr;
//	system_set_os_print(uart_nr >= 0);
//	ets_install_putc1(smg_uart_debug_putc);
}

int smg_uart_get_debug()
{
	return s_uart_debug_nr;
}

uint8_t smg_uart_disable_interrupts()
{
	// TODO:...
	return 0;
}

void smg_uart_restore_interrupts()
{
	// TODO: ...
}
