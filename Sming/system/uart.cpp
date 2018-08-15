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

#include "esp_systemapi.h"
#include "espinc/uart.h"
#include "espinc/peri.h"
#include <user_interface.h>

static int s_uart_debug_nr = UART0;

size_t uart_resize_rx_buffer(uart_t* uart, size_t new_size)
{
    if (!uart || !uart->rx_enabled)
        return 0;

    auto buf = uart->rx_buffer;
    if (buf->size == new_size)
        return buf->size;

    uint8_t* new_buf = new uint8_t[new_size];
    if (!new_buf)
        return buf->size;

    size_t new_wpos = 0;
    ETS_UART_INTR_DISABLE();
    while (uart_rx_available(uart) && new_wpos < new_size)
        new_buf[new_wpos++] = uart_read_char(uart);

    delete[] buf->buffer;
    buf->buffer = new_buf;
    buf->size = new_size;
    buf->rpos = 0;
    buf->wpos = new_wpos;
    ETS_UART_INTR_ENABLE();
    return buf->size;
}


int uart_peek_char(uart_t* uart)
{
    if (!uart || !uart->rx_enabled)
        return -1;

    if (!uart_rx_available(uart))
        return -1;

    auto buf = uart->rx_buffer;
    return buf->buffer[buf->rpos];
}

int uart_read_char(uart_t* uart)
{
    int data = uart_peek_char(uart);
    if (data >= 0) {
        auto buf = uart->rx_buffer;
        buf->rpos = (buf->rpos + 1) % buf->size;
    }
    return data;
}

size_t uart_rx_available(uart_t* uart)
{
    if (!uart || !uart->rx_enabled)
        return 0;

    auto buf = uart->rx_buffer;
	int ret = buf->wpos - buf->rpos;
	if (ret < 0)
		ret += buf->size;
	return ret;
}


void IRAM_ATTR uart_isr(void * arg)
{
    auto uart = (uart_t*)arg;
    if (!uart || !uart->rx_enabled) {
        USIC(uart->uart_nr) = USIS(uart->uart_nr);
        ETS_UART_INTR_DISABLE();
        return;
    }

    if (USIS(uart->uart_nr) & ((1 << UIFF) | (1 << UITO))) {
        while ((USS(uart->uart_nr) >> USRXC) & 0x7F) {
            uint8_t data = USF(uart->uart_nr);
            size_t nextPos = (uart->rx_buffer->wpos + 1) % uart->rx_buffer->size;
            if (nextPos != uart->rx_buffer->rpos) {
                uart->rx_buffer->buffer[uart->rx_buffer->wpos] = data;
                uart->rx_buffer->wpos = nextPos;
            }
        }
    }

    USIC(uart->uart_nr) = USIS(uart->uart_nr);

    if (uart->callback)
    	uart->callback(uart);
}


void uart_start_isr(uart_t* uart)
{
    if (!uart || !uart->rx_enabled)
    	return;

    USC1(uart->uart_nr) = (127 << UCFFT) | (0x02 << UCTOT) | (1 <<UCTOE );
	USIC(uart->uart_nr) = 0xffff;
	USIE(uart->uart_nr) = (1 << UIFF) | (1 << UIFR) | (1 << UITO);
	ETS_UART_INTR_ATTACH(uart_isr,  (void *)uart);
	ETS_UART_INTR_ENABLE();
}

void uart_stop_isr(uart_t* uart)
{
    if (!uart || !uart->rx_enabled)
    	return;

	ETS_UART_INTR_DISABLE();
	USC1(uart->uart_nr) = 0;
	USIC(uart->uart_nr) = 0xffff;
	USIE(uart->uart_nr) = 0;
	ETS_UART_INTR_ATTACH(nullptr, nullptr);
}


void uart_write_char(uart_t* uart, char c)
{
    if (!uart || !uart->tx_enabled)
    	return;

	while((USS(uart->uart_nr) >> USTXC) >= 0x7f)
		;
	USF(uart->uart_nr) = c;
}


void uart_write(uart_t* uart, const char* buf, size_t size)
{
    if (!uart || !uart->tx_enabled)
    	return;

	while(size--)
		uart_write_char(uart, *buf++);
}


size_t uart_tx_free(uart_t* uart)
{
    if (!uart || !uart->tx_enabled)
        return 0;

    return UART_TX_FIFO_SIZE - ((USS(uart->uart_nr) >> USTXC) & 0xff);
}


void uart_wait_tx_empty(uart_t* uart)
{
    if (!uart || !uart->tx_enabled)
        return;

    while(((USS(uart->uart_nr) >> USTXC) & 0xff) > 0)
        delay(0);
}


void uart_flush(uart_t* uart)
{
    if (!uart)
        return;

    uint32_t tmp = 0x00000000;
    if (uart->rx_enabled) {
        tmp |= (1 << UCRXRST);
        ETS_UART_INTR_DISABLE();
        uart->rx_buffer->rpos = 0;
        uart->rx_buffer->wpos = 0;
        ETS_UART_INTR_ENABLE();
    }

    if (uart->tx_enabled)
        tmp |= (1 << UCTXRST);

    USC0(uart->uart_nr) |= (tmp);
    USC0(uart->uart_nr) &= ~(tmp);
}


void uart_set_baudrate(uart_t* uart, int baud_rate)
{
    if (!uart)
        return;

    uart->baud_rate = baud_rate;
    USD(uart->uart_nr) = (ESP8266_CLOCK / uart->baud_rate);
}


int uart_get_baudrate(uart_t* uart)
{
    return uart ? uart->baud_rate : 0;
}


uart_t* uart_init(int uart_nr, int baudrate, int config, int mode, int tx_pin, size_t rx_size)
{
    auto uart = new uart_t;
    if (!uart)
        return nullptr;

    uart->uart_nr = uart_nr;

    switch(uart->uart_nr) {
    case UART0:
        ETS_UART_INTR_DISABLE();
        ETS_UART_INTR_ATTACH(nullptr, nullptr);
        uart->rx_enabled = (mode != UART_TX_ONLY);
        uart->tx_enabled = (mode != UART_RX_ONLY);
        uart->rx_pin = (uart->rx_enabled)?3:255;
        if (uart->rx_enabled) {
            auto rx_buffer = new uart_rx_buffer_;
            if (!rx_buffer) {
              delete uart;
              return nullptr;
            }
            rx_buffer->size = rx_size;//var this
            rx_buffer->rpos = 0;
            rx_buffer->wpos = 0;
            rx_buffer->buffer = new uint8_t[rx_buffer->size];
            if (!rx_buffer->buffer) {
              delete[] rx_buffer;
              delete uart;
              return nullptr;
            }
            uart->rx_buffer = rx_buffer;
            pinMode(uart->rx_pin, SPECIAL);
        }

        if (uart->tx_enabled) {
            if (tx_pin == 2) {
                uart->tx_pin = 2;
                pinMode(uart->tx_pin, FUNCTION_4);
            }
            else {
                uart->tx_pin = 1;
                pinMode(uart->tx_pin, FUNCTION_0);
            }
        }
        else
            uart->tx_pin = 255;

        IOSWAP &= ~(1 << IOSWAPU0);
        break;

    case UART1:
        // Note: uart_interrupt_handler does not support RX on UART 1.
        uart->rx_enabled = false;
        uart->tx_enabled = (mode != UART_RX_ONLY);
        uart->rx_pin = 255;
        // GPIO7 as TX not possible! See GPIO pins used by UART
        uart->tx_pin = (uart->tx_enabled) ? 2 : 255;
        if (uart->tx_enabled)
            pinMode(uart->tx_pin, SPECIAL);
        break;

    case UART_NO:
    default:
        // big fail!
        delete uart;
        return nullptr;
    }

    uart_set_baudrate(uart, baudrate);
    USC0(uart->uart_nr) = config;
    uart_flush(uart);
    USC1(uart->uart_nr) = 0;
    USIC(uart->uart_nr) = 0xffff;
    USIE(uart->uart_nr) = 0;
    if (uart->uart_nr == UART0 && uart->rx_enabled)
        uart_start_isr(uart);

    return uart;
}


void uart_uninit(uart_t* uart)
{
    if (!uart)
        return;

    switch (uart->rx_pin) {
    case 3:
        pinMode(3, INPUT);
        break;

    case 13:
        pinMode(13, INPUT);
        break;
    }

    switch (uart->tx_pin) {
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

    if (uart->rx_enabled){
        delete[] uart->rx_buffer->buffer;
        delete[] uart->rx_buffer;
        uart_stop_isr(uart);
    }

    delete uart;
}


void uart_swap(uart_t* uart, int tx_pin)
{
    if (!uart)
        return;

    switch(uart->uart_nr) {
    case UART0:
        if (((uart->tx_pin == 1 || uart->tx_pin == 2) && uart->tx_enabled) || (uart->rx_pin == 3 && uart->rx_enabled)) {
            if (uart->tx_enabled) {
                pinMode(uart->tx_pin, INPUT);
                uart->tx_pin = 15;
            }

            if (uart->rx_enabled) {
                pinMode(uart->rx_pin, INPUT);
                uart->rx_pin = 13;
            }

            if (uart->tx_enabled)
                pinMode(uart->tx_pin, FUNCTION_4);

            if (uart->rx_enabled)
                pinMode(uart->rx_pin, FUNCTION_4);

            IOSWAP |= (1 << IOSWAPU0);
        }
        else {
            if (uart->tx_enabled) {
                pinMode(uart->tx_pin, INPUT);
                uart->tx_pin = (tx_pin == 2)?2:1;
            }

            if (uart->rx_enabled) {
                pinMode(uart->rx_pin, INPUT);
                uart->rx_pin = 3;
            }

            if (uart->tx_enabled)
                pinMode(uart->tx_pin, (tx_pin == 2) ? FUNCTION_4 : SPECIAL);

            if (uart->rx_enabled)
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
    if (!uart)
        return;

    switch(uart->uart_nr) {
    case UART0:
        if (uart->tx_enabled) {
            if (uart->tx_pin == 1 && tx_pin == 2) {
                pinMode(uart->tx_pin, INPUT);
                uart->tx_pin = 2;
                pinMode(uart->tx_pin, FUNCTION_4);
            }
            else if (uart->tx_pin == 2 && tx_pin != 2) {
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
    if (!uart)
        return;

    // Only UART0 allows pin changes
    if (uart->uart_nr == UART0) {
        if (uart->tx_enabled && uart->tx_pin != tx) {
            if ( rx == 13 && tx == 15)
                uart_swap(uart, 15);
            else if (rx == 3 && (tx == 1 || tx == 2)) {
                if (uart->rx_pin != rx)
                    uart_swap(uart, tx);
                else
                    uart_set_tx(uart, tx);
            }
        }
        if (uart->rx_enabled && uart->rx_pin != rx && rx == 13 && tx == 15)
            uart_swap(uart, 15);
    }
}


bool uart_tx_enabled(uart_t* uart)
{
    return uart ? uart->tx_enabled : false;
}


bool uart_rx_enabled(uart_t* uart)
{
    return uart ? uart->rx_enabled : false;
}


static void uart_ignore_char(char c)
{
}


static void uart0_write_char(char c)
{
    while(((USS(0) >> USTXC) & 0xff) >= 0x7F)
        delay(0);

    USF(0) = c;
}


static void uart1_write_char(char c)
{
    while(((USS(1) >> USTXC) & 0xff) >= 0x7F)
        delay(0);

    USF(1) = c;
}


void uart_set_debug(int uart_nr)
{
    s_uart_debug_nr = uart_nr;
    switch(s_uart_debug_nr) {
    case UART0:
        system_set_os_print(1);
        ets_install_putc1(uart0_write_char);
        break;

    case UART1:
        system_set_os_print(1);
        ets_install_putc1(uart1_write_char);
        break;

    case UART_NO:
    default:
        system_set_os_print(0);
        ets_install_putc1(uart_ignore_char);
        break;
    }
}


int uart_get_debug()
{
    return s_uart_debug_nr;
}


void uart_detach(int uart_nr)
{
	ETS_UART_INTR_DISABLE();
	USC1(uart_nr) = 0;
	USIC(uart_nr) = 0xffff;
	USIE(uart_nr) = 0;
	ETS_UART_INTR_ATTACH(nullptr, nullptr);
}


