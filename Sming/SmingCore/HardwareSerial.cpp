/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// HardwareSerial based on Espressif Systems code
#include "HardwareSerial.h"
#include "WiringFrameworkIncludes.h"
#include <cstdarg>

#include "Clock.h"
#include "Interrupts.h"

#include "m_printf.h"

HWSerialMemberData HardwareSerial::_memberData[NUMBER_UARTS];
os_event_t* HardwareSerial::_serialQueue = nullptr;
bool HardwareSerial::_init = false;

HardwareSerial Serial(UART_ID_0);

#define DEFAULT_RX_BUFFER_SIZE 256

HardwareSerial::HardwareSerial(const int uartPort) : _uartNr(uartPort), _rxSize(DEFAULT_RX_BUFFER_SIZE)
{}

HardwareSerial::~HardwareSerial()
{}

void HardwareSerial::begin(const uint32_t baud, SerialConfig config, SerialMode mode, uint8_t txPin)
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

	uint8_t lastPos = uart->rx_buffer->wpos;
	if (lastPos == 0)
		lastPos = uart->rx_buffer->size;

	uint8_t receivedChar = uart->rx_buffer->buffer[lastPos - 1];
	if ((_memberData[uart->uart_nr].HWSDelegate)
#if ENABLE_CMD_EXECUTOR
		|| (_memberData[uart->uart_nr].commandExecutor)
#endif
	) {
		uint32 serialQueueParameter;
		uint16 cc = uart_rx_available(uart);
		/*
		 * Can be done by bitlogic, avoid casting to ETSParam
		 * the left most byte contains the uart_nr. Up to 256 uarts are supported
		 */
		serialQueueParameter = (uart->uart_nr << 25) | (cc << 8) | receivedChar;

		if (_memberData[uart->uart_nr].HWSDelegate)
			system_os_post(USER_TASK_PRIO_0, SERIAL_SIGNAL_DELEGATE, serialQueueParameter);

#if ENABLE_CMD_EXECUTOR
		if (_memberData[uart->uart_nr].commandExecutor)
			system_os_post(USER_TASK_PRIO_0, SERIAL_SIGNAL_COMMAND, serialQueueParameter);
#endif
	}
}

bool HardwareSerial::setCallback(StreamDataReceivedDelegate reqDelegate)
{
	if (!_uart || !uart_rx_enabled(_uart))
		return false;

	_uart->callback = callbackHandler;

	_memberData[_uartNr].HWSDelegate = reqDelegate;

	// Start Serial task
	if (!_serialQueue) {
		_serialQueue = new os_event_t[SERIAL_QUEUE_LEN];
		system_os_task(delegateTask, USER_TASK_PRIO_0, _serialQueue, SERIAL_QUEUE_LEN);
	}

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
	if (reqEnable) {
		if (!_memberData[_uartNr].commandExecutor)
			_memberData[_uartNr].commandExecutor = new CommandExecutor(&Serial);
	}
	else {
		delete _memberData[_uartNr].commandExecutor;
		_memberData[_uartNr].commandExecutor = nullptr;
	}
#endif
}

void HardwareSerial::delegateTask(os_event_t* inputEvent)
{
	// the uart_nr is in the last byte
	int uartNr = inputEvent->par >> 25;
	// clear the last byte
	inputEvent->par &= 0x00FFFFFF;
	// can be done by bitlogic, avoid casting from ETSParam
	uint16 charCount = inputEvent->par >> 8;
	uint8 rcvChar = inputEvent->par & 0xFF;

	switch (inputEvent->sig) {
	case SERIAL_SIGNAL_DELEGATE:
		if (_memberData[uartNr].HWSDelegate)
			_memberData[uartNr].HWSDelegate(Serial, rcvChar, charCount);
		break;

	case SERIAL_SIGNAL_COMMAND:
#if ENABLE_CMD_EXECUTOR
		if (_memberData[uartNr].commandExecutor)
			_memberData[uartNr].commandExecutor->executorReceive(rcvChar);
#endif
		break;

	default:
		break;
	}
}

int HardwareSerial::baudRate(void)
{
	// Null pointer on _uart is checked by SDK
	return uart_get_baudrate(_uart);
}

HardwareSerial::operator bool() const
{
	return _uart != nullptr;
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
