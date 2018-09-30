/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// HardwareSerial based on Espressif Systems code

#include "../SmingCore/HardwareSerial.h"
#include "../Wiring/WiringFrameworkIncludes.h"
#include <cstdarg>

#include "../SmingCore/Clock.h"
#include "../SmingCore/Interrupts.h"

HWSerialMemberData HardwareSerial::memberData[NUMBER_UARTS];
os_event_t* HardwareSerial::serialQueue = nullptr;
bool HardwareSerial::init = false;

HardwareSerial::HardwareSerial(const int uartPort) : uartNr(uartPort), rxSize(256)
{
}

HardwareSerial::~HardwareSerial()
{
}

void HardwareSerial::begin(const uint32_t baud, SerialConfig config, SerialMode mode, uint8_t txPin)
{
	end();
	uart = uart_init(uartNr, baud, (int)config, (int)mode, txPin, rxSize);
	resetCallback();
}

void HardwareSerial::end()
{
	if(uart_get_debug() == uartNr) {
		uart_set_debug(UART_NO);
	}

	if(uart) {
		uart_uninit(uart);
		uart = NULL;
	} else if(!init) {
		uart_detach(uartNr);
		init = true;
	}
}

size_t HardwareSerial::setRxBufferSize(size_t size)
{
	if(uart) {
		rxSize = uart_resize_rx_buffer(uart, size);
	} else {
		rxSize = size;
	}
	return rxSize;
}

void HardwareSerial::swap(uint8_t tx_pin)
{
	if(!uart) {
		return;
	}
	uart_swap(uart, tx_pin);
}

void HardwareSerial::setTx(uint8_t tx_pin)
{
	if(!uart) {
		return;
	}
	uart_set_tx(uart, tx_pin);
}

void HardwareSerial::pins(uint8_t tx, uint8_t rx)
{
	if(!uart) {
		return;
	}
	uart_set_pins(uart, tx, rx);
}

bool HardwareSerial::isTxEnabled(void)
{
	return uart && uart_tx_enabled(uart);
}

bool HardwareSerial::isRxEnabled(void)
{
	return uart && uart_rx_enabled(uart);
}

int HardwareSerial::available()
{
	int result = static_cast<int>(uart_rx_available(uart));
	return result;
}

size_t HardwareSerial::write(uint8_t oneChar)
{
	if(!uart || !uart_tx_enabled(uart)) {
		return 0;
	}

	uart_write_char(uart, oneChar);
	return 1;
}

int HardwareSerial::read()
{
	return uart_read_char(uart);
}

int HardwareSerial::readMemoryBlock(char* buf, int max_len)
{
	if(uart != 0 && uart_rx_enabled(uart)) {
		int size = 0;
		char c;
		for(int i = 0; i < max_len; i++) {
			c = read();
			if(c == -1) {
				break;
			}

			size++;
			// @TODO: Check this section...
			buf[i] = c;
		}
		return size;
	}
	return -1;
}

int HardwareSerial::peek()
{
	return uart_peek_char(uart);
}

void HardwareSerial::flush()
{
	if(!uart || !uart_tx_enabled(uart)) {
		return;
	}

	uart_wait_tx_empty(uart);
}

void HardwareSerial::systemDebugOutput(bool enabled)
{
	if(!uart) {
		return;
	}

	if(enabled) {
		if(uart_tx_enabled(uart)) {
			uart_set_debug(uartNr);
			m_setPuts(reinterpret_cast<nputs_callback_t>(uart_write), uart);
		} else {
			uart_set_debug(UART_NO);
		}
	} else {
		// don't print debugf() data at all
		m_setPuts(nullptr, nullptr);
		// and disable system debug messages on this interface
		if(uart_get_debug() == uartNr) {
			uart_set_debug(UART_NO);
		}
	}
}

void HardwareSerial::callbackHandler(uart_t* uart)
{
	if(uart == NULL) {
		return;
	}

	uint8_t lastPos = uart->rx_buffer->wpos;
	if(!lastPos) {
		lastPos = uart->rx_buffer->size;
	}
	uint8_t receivedChar = uart->rx_buffer->buffer[lastPos - 1];
	if((memberData[uart->uart_nr].HWSDelegate)
#if ENABLE_CMD_EXECUTOR
	   || (memberData[uart->uart_nr].commandExecutor)
#endif
	) {
		uint32 serialQueueParameter;
		uint16 cc = uart_rx_available(uart);
		serialQueueParameter = (cc * 256) + receivedChar; // can be done by bitlogic, avoid casting to ETSParam
		serialQueueParameter +=
			(uart->uart_nr << 25); // the left most byte contains the uart_nr. Up to 256 uarts are supported

		if(memberData[uart->uart_nr].HWSDelegate) {
			system_os_post(USER_TASK_PRIO_0, SERIAL_SIGNAL_DELEGATE, serialQueueParameter);
		}
#if ENABLE_CMD_EXECUTOR
		if(memberData[uart->uart_nr].commandExecutor) {
			system_os_post(USER_TASK_PRIO_0, SERIAL_SIGNAL_COMMAND, serialQueueParameter);
		}
#endif
	}
}

bool HardwareSerial::setCallback(StreamDataReceivedDelegate reqDelegate)
{
	if(!uart || !uart_rx_enabled(uart)) {
		return false;
	}

	uart->callback = callbackHandler;

	memberData[uartNr].HWSDelegate = reqDelegate;

	// Start Serial task
	if(!serialQueue) {
		serialQueue = (os_event_t*)malloc(sizeof(os_event_t) * SERIAL_QUEUE_LEN);
		system_os_task(delegateTask, USER_TASK_PRIO_0, serialQueue, SERIAL_QUEUE_LEN);
	}

	return true;
}

void HardwareSerial::resetCallback()
{
	memberData[uartNr].HWSDelegate = nullptr;

	uart->callback = 0;
}

void HardwareSerial::commandProcessing(bool reqEnable)
{
#if ENABLE_CMD_EXECUTOR
	if(reqEnable) {
		if(!memberData[uartNr].commandExecutor) {
			memberData[uartNr].commandExecutor = new CommandExecutor(&Serial);
		}
	} else {
		delete memberData[uartNr].commandExecutor;
		memberData[uartNr].commandExecutor = nullptr;
	}
#endif
}

void HardwareSerial::delegateTask(os_event_t* inputEvent)
{
	int uartNr = inputEvent->par >> 25;				// the uart_nr is in the last byte
	inputEvent->par = inputEvent->par & 0x00FFFFFF; // clear the last byte
	uint8 rcvChar = inputEvent->par % 256;			// can be done by bitlogic, avoid casting from ETSParam
	uint16 charCount = inputEvent->par / 256;

	switch(inputEvent->sig) {
	case SERIAL_SIGNAL_DELEGATE:

		if(memberData[uartNr].HWSDelegate) {
			memberData[uartNr].HWSDelegate(Serial, rcvChar, charCount);
		}
		break;

	case SERIAL_SIGNAL_COMMAND:
#if ENABLE_CMD_EXECUTOR
		if(memberData[uartNr].commandExecutor) {
			memberData[uartNr].commandExecutor->executorReceive(rcvChar);
		}
#endif
		break;

	default:
		break;
	}
}

int HardwareSerial::baudRate(void)
{
	// Null pointer on _uart is checked by SDK
	return uart_get_baudrate(uart);
}

HardwareSerial::operator bool() const
{
	return uart != 0;
}

size_t HardwareSerial::indexOf(char c)
{
	int offset = uart->rx_buffer->rpos;
	int pos = 0;
	while(pos < available()) {
		if(uart->rx_buffer->buffer[offset + pos] == c) {
			return pos;
		}

		pos++;

		if(pos + offset == uart->rx_buffer->wpos) {
			break;
		}

		if(pos + offset == uart->rx_buffer->size) {
			offset = -pos;
		}
	}

	return -1;
}

HardwareSerial Serial(UART_ID_0);
