#include "include/driver/uart.h"
#include "include/driver/SerialBuffer.h"
#include <BitManipulations.h>
#include <esp_systemapi.h>

bool smg_uart_realloc_buffer(SerialBuffer*& buffer, size_t new_size)
{
	if(buffer != nullptr) {
		size_t res = 0;
		smg_uart_disable_interrupts();
		if(new_size == 0) {
			delete buffer;
			buffer = nullptr;
		} else {
			res = buffer->resize(new_size);
		}
		smg_uart_restore_interrupts();

		return res == new_size;
	}

	if(new_size == 0) {
		return true;
	}

#ifdef ARCH_ESP32
	// Avoid allocating in SPIRAM
	auto mem = heap_caps_malloc(sizeof(SerialBuffer), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
	auto new_buf = new(mem) SerialBuffer;
#else
	auto new_buf = new SerialBuffer;
#endif
	if(new_buf != nullptr && new_buf->resize(new_size) == new_size) {
		buffer = new_buf;
		return true;
	}

	delete new_buf;
	return false;
}

smg_uart_t* smg_uart_init(uint8_t uart_nr, uint32_t baudrate, smg_uart_format_t config, smg_uart_mode_t mode,
						  uint8_t tx_pin, size_t rx_size, size_t tx_size)
{
	smg_uart_config_t cfg = {
		.uart_nr = uart_nr,
		.tx_pin = tx_pin,
		.rx_pin = UART_PIN_DEFAULT,
		.mode = mode,
		.options = _BV(UART_OPT_TXWAIT),
		.baudrate = baudrate,
		.config = config,
		.rx_size = rx_size,
		.tx_size = tx_size,
	};
	return smg_uart_init_ex(cfg);
}

size_t smg_uart_resize_rx_buffer(smg_uart_t* uart, size_t new_size)
{
	if(smg_uart_rx_enabled(uart)) {
		smg_uart_realloc_buffer(uart->rx_buffer, new_size);
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
		smg_uart_realloc_buffer(uart->tx_buffer, new_size);
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
