/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// HardwareSerial based on Espressif Systems code
#include "HardwareSerial.h"
#include <cstdarg>

#include "System.h"
#include "Interrupts.h"

#include "m_printf.h"

HWSerialMemberData HardwareSerial::_memberData[NUMBER_UARTS];
bool HardwareSerial::_init = false;

HardwareSerial Serial(UART_ID_0);

void HardwareSerial::begin(uint32_t baud, SerialConfig config, SerialMode mode, uint8_t txPin)
{
	end();
	_uart = uart_init(_uartNr, baud, (int)config, (int)mode, txPin, _rxSize);
	resetCallback();
}

void HardwareSerial::end()
{
	if (uart_get_debug() == _uartNr)
		uart_set_debug(UART_NO);

	if (_uart) {
		uart_uninit(_uart);
		_uart = nullptr;
	}
	else if (!_init) {
		uart_detach(_uartNr);
		_init = true;
	}
}

size_t HardwareSerial::setRxBufferSize(size_t size)
{
	if (_uart) {
		_rxSize = uart_resize_rx_buffer(_uart, size);
	}
	else {
		_rxSize = size;
	}
	return _rxSize;
}

void HardwareSerial::swap(uint8_t tx_pin)
{
	if (_uart)
		uart_swap(_uart, tx_pin);
}

void HardwareSerial::setTx(uint8_t tx_pin)
{
	if (_uart)
		uart_set_tx(_uart, tx_pin);
}

void HardwareSerial::pins(uint8_t tx, uint8_t rx)
{
	if (_uart)
		uart_set_pins(_uart, tx, rx);
}

bool HardwareSerial::isTxEnabled(void)
{
	return _uart && uart_tx_enabled(_uart);
}

bool HardwareSerial::isRxEnabled(void)
{
	return _uart && uart_rx_enabled(_uart);
}

int HardwareSerial::available()
{
	return (int)uart_rx_available(_uart);
}

size_t HardwareSerial::write(uint8_t oneChar)
{
	if (!_uart || !uart_tx_enabled(_uart))
		return 0;

	uart_write_char(_uart, oneChar);
	return 1;
}

int HardwareSerial::read()
{
	return uart_read_char(_uart);
}

size_t HardwareSerial::readMemoryBlock(char* buf, size_t max_len)
{
	if (!_uart || !uart_rx_enabled(_uart))
		return 0;

	for (unsigned i = 0; i < max_len; i++) {
		char c = read();
		if (c < 0)
			return i;

		buf[i] = c;
	}

	return max_len;
}

int HardwareSerial::peek()
{
	return uart_peek_char(_uart);
}

void HardwareSerial::flush()
{
	if (_uart && uart_tx_enabled(_uart))
		uart_wait_tx_empty(_uart);
}

void HardwareSerial::systemDebugOutput(bool enabled)
{
	if (!_uart)
		return;

	if (enabled) {
		if (uart_tx_enabled(_uart)) {
			uart_set_debug(_uartNr);
			m_setPutchar((putchar_callback_t)uart_write_char, _uart);
		}
		else
			uart_set_debug(UART_NO);
	}
	else {
		// don't print debugf() data at all
		m_setPutchar(nullptr, nullptr);
		// and disable system debug messages on this interface
		if (uart_get_debug() == _uartNr)
			uart_set_debug(UART_NO);
	}
}

void HardwareSerial::callbackHandler(uart_t* uart)
{
	if (!uart)
		return;

	auto& buf = *uart->rx_buffer;
	uint8_t lastPos = buf.wpos;
	if (lastPos == 0)
		lastPos = buf.size;

	uint8_t receivedChar = buf.buffer[lastPos - 1];

	auto& m = _memberData[uart->uart_nr];

#if ENABLE_CMD_EXECUTOR
	if (m.HWSDelegate || m.commandExecutor) {
#else
	if (m.HWSDelegate) {
#endif

		// Pack parameters for callback into a single word
		union __packed SerialParam {
			struct
			{
				uint16_t charCount;
				uint8_t receivedChar;
				uint8_t uartNr;
			};
			os_param_t param;
		};

		SerialParam ser = {{.charCount = (uint16_t)uart_rx_available(uart),
							.receivedChar = receivedChar,
							.uartNr = (uint8_t)uart->uart_nr}};

		if (m.HWSDelegate)
			System.deferCallback(
				[](os_param_t param) {
					SerialParam ser = {.param = param};
					auto& m = _memberData[ser.uartNr];
					if (m.HWSDelegate)
						m.HWSDelegate(Serial, ser.receivedChar, ser.charCount);
				},
				ser.param);

#if ENABLE_CMD_EXECUTOR
		if (m.commandExecutor)
			System.deferCallback(
				[](os_param_t param) {
					SerialParam ser = {.param = param};
					auto& m = _memberData[ser.uartNr];
					if (m.commandExecutor)
						m.commandExecutor->executorReceive(ser.receivedChar);
				},
				ser.param);

#endif
	}
}

bool HardwareSerial::setCallback(StreamDataReceivedDelegate reqDelegate)
{
	if (!_uart || !uart_rx_enabled(_uart))
		return false;

	_uart->callback = callbackHandler;
	_memberData[_uartNr].HWSDelegate = reqDelegate;

	return true;
}

void HardwareSerial::resetCallback()
{
	_memberData[_uartNr].HWSDelegate = nullptr;
	_uart->callback = 0;
}

void HardwareSerial::commandProcessing(bool reqEnable)
{
#if ENABLE_CMD_EXECUTOR
	auto& m = _memberData[_uartNr];
	if (reqEnable) {
		if (!m.commandExecutor)
			m.commandExecutor = new CommandExecutor(&Serial);
	}
	else {
		delete m.commandExecutor;
		m.commandExecutor = nullptr;
	}
#endif
}

int HardwareSerial::baudRate(void)
{
	// Null pointer on _uart is checked by SDK
	return uart_get_baudrate(_uart);
}

int HardwareSerial::indexOf(char c)
{
	unsigned offset = _uart->rx_buffer->rpos;
	unsigned pos = 0;
	while ((int)pos < available()) {
		if (_uart->rx_buffer->buffer[offset + pos] == c)
			return pos;

		pos++;

		if (pos + offset == _uart->rx_buffer->wpos)
			break;

		if (pos + offset == _uart->rx_buffer->size)
			offset = -pos;
	}

	return -1;
}
