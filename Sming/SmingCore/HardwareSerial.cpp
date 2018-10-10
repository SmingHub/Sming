/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// HardwareSerial based on Espressif Systems code

#include "HardwareSerial.h"
#include <cstdarg>

#include "../SmingCore/Clock.h"
#include "../SmingCore/Interrupts.h"

HWSerialMemberData HardwareSerial::memberData[NUMBER_UARTS];
bool HardwareSerial::init = false;

HardwareSerial::HardwareSerial(const int uartPort) : uartNr(uartPort), rxSize(DEFAULT_RX_BUFFER_SIZE)
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
		uart = nullptr;
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
	return uart_tx_enabled(uart);
}

bool HardwareSerial::isRxEnabled(void)
{
	return uart_rx_enabled(uart);
}

int HardwareSerial::available()
{
	int result = static_cast<int>(uart_rx_available(uart));
	return result;
}

int HardwareSerial::read()
{
	return uart_read_char(uart);
}

// @todo This does not comply with spec. as it affects stream position; we cannot seek on the stream without some trouble
int HardwareSerial::readMemoryBlock(char* buf, int max_len)
{
	if(uart_rx_enabled(uart)) {
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
	if(!uart_tx_enabled(uart)) {
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
			using namespace std::placeholders;
			m_setPuts(std::bind(&uart_write, uart, _1, _2));
		} else {
			uart_set_debug(UART_NO);
		}
	} else {
		// don't print debugf() data at all
		m_setPuts(nullptr);
		// and disable system debug messages on this interface
		if(uart_get_debug() == uartNr) {
			uart_set_debug(UART_NO);
		}
	}
}

void HardwareSerial::callbackHandler(uart_t* uart)
{
	if(uart == nullptr || uart->uart_nr < 0) {
		return;
	}

	int uart_nr = uart->uart_nr;
	auto& m = memberData[uart_nr];

	uint8_t lastPos = uart->rx_buffer->wpos;
	if(!lastPos) {
		lastPos = uart->rx_buffer->size;
	}
	uint8_t receivedChar = uart->rx_buffer->buffer[lastPos - 1];
	if((m.HWSDelegate)
#if ENABLE_CMD_EXECUTOR
	   || (m.commandExecutor)
#endif
	) {
		// Pack parameters for callback into a single word
		union __packed SerialParam {
			struct {
				uint16_t charCount;
				uint8_t receivedChar;
				uint8_t uartNr;
			};
			os_param_t param;
		};

		SerialParam ser = {{.charCount = static_cast<uint16_t>(uart_rx_available(uart)),
							.receivedChar = receivedChar,
							.uartNr = static_cast<uint8_t>(uart_nr)}};

		if(m.HWSDelegate)
			System.queueCallback(
				[](os_param_t param) {
					SerialParam ser = {.param = param};
					auto& m = memberData[ser.uartNr];
					if(m.HWSDelegate)
						m.HWSDelegate(Serial, ser.receivedChar, ser.charCount);
				},
				ser.param);

#if ENABLE_CMD_EXECUTOR
		if(m.commandExecutor)
			System.queueCallback(
				[](os_param_t param) {
					SerialParam ser = {.param = param};
					auto& m = memberData[ser.uartNr];
					if(m.commandExecutor)
						m.commandExecutor->executorReceive(ser.receivedChar);
				},
				ser.param);

#endif
	}
}

bool HardwareSerial::setCallback(StreamDataReceivedDelegate reqDelegate)
{
	if(!uart_rx_enabled(uart)) {
		return false;
	}

	uart->callback = callbackHandler;

	memberData[uartNr].HWSDelegate = reqDelegate;
	return true;
}

void HardwareSerial::resetCallback()
{
	memberData[uartNr].HWSDelegate = nullptr;

	uart->callback = nullptr;
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

int HardwareSerial::baudRate(void)
{
	// Null pointer on _uart is checked by SDK
	return uart_get_baudrate(uart);
}

HardwareSerial::operator bool() const
{
	return uart != nullptr;
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
