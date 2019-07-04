/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdbuart.cpp
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "gdbuart.h"
#include "GdbPacket.h"
#include "driver/uart.h"
#include "driver/SerialBuffer.h"
#include "Platform/System.h"
#include "HardwareSerial.h"
#include "HardwareTimer.h"
#include "gdbsyscall.h"

#define GDB_UART UART0 // Only UART0 supports for debugging as RX/TX required

static uart_t* gdb_uart; // Port debugger is attached to

#if GDBSTUB_ENABLE_UART2
static uart_t* user_uart;			  // If open, virtual port being used for user passthrough
static uint32_t user_uart_status;	 // See gdb_uart_callback (ISR handler)
static volatile bool userDataSending; // Transmit completion callback invoked on user uart
static bool sendUserDataQueued;		  // Ensures only one call to gdbSendUserData() is queued at a time
static uint8_t break_requests;		  ///< How many times Ctrl+C was received before actually breaking
#endif

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

/*
 * Minimal version of uart_read_char()
 */
static int ATTR_GDBEXTERNFN gdb_uart_read_char()
{
	int c;
	if(uart_rxfifo_count(GDB_UART) == 0) {
		c = -1;
	} else {
		c = USF(GDB_UART) & 0xff;
	}
	return c;
}

/*
 * Minimal version of uart_write(). Blocks until all data queued.
 */
static size_t ATTR_GDBEXTERNFN gdb_uart_write(const void* data, size_t length)
{
	if(data == nullptr || length == 0) {
		return 0;
	}

	for(unsigned i = 0; i < length; ++i) {
		while(uart_txfifo_full(GDB_UART)) {
			//
		}
		USF(GDB_UART) = static_cast<const uint8_t*>(data)[i];
	}

	// Enable TX FIFO EMPTY interrupt
	bitSet(USIE(GDB_UART), UIFE);

	return length;
}

/*
 * Minimal version of uart_write_char(). Blocks until data queued.
 */
static size_t ATTR_GDBEXTERNFN gdb_uart_write_char(char c)
{
	while(uart_txfifo_full(GDB_UART)) {
		//
	}
	USF(GDB_UART) = c;

	// Enable TX FIFO EMPTY interrupt
	bitSet(USIE(GDB_UART), UIFE);

	return 1;
}

int ATTR_GDBEXTERNFN gdbReceiveChar()
{
#if GDBSTUB_UART_READ_TIMEOUT
	auto timeout = usToTimerTicks(GDBSTUB_UART_READ_TIMEOUT * 1000U);
	auto startTicks = NOW();
#define checkTimeout() (NOW() - startTicks >= timeout)
#else
#define checkTimeout() (false)
#endif

	do {
		wdt_feed();
		system_soft_wdt_feed();
		int c = gdb_uart_read_char();
		if(c >= 0) {
#if GDBSTUB_ENABLE_DEBUG >= 3
			m_putc(c);
#endif
			return c;
		}
	} while(!checkTimeout());

	return -1;
}

size_t ATTR_GDBEXTERNFN gdbSendData(const void* data, size_t length)
{
#if GDBSTUB_ENABLE_DEBUG >= 3
	m_nputs(static_cast<const char*>(data), length);
#endif
	return gdb_uart_write(data, length);
}

size_t ATTR_GDBEXTERNFN gdbSendChar(char c)
{
#if GDBSTUB_ENABLE_DEBUG >= 3
	m_putc(c);
#endif
	return gdb_uart_write_char(c);
}

size_t ATTR_GDBEXTERNFN gdbWriteConsole(const char* data, size_t length)
{
	GdbPacket packet;
	packet.writeChar('O');
	packet.writeHexBlock(data, length);
	return length;
}

size_t ATTR_GDBEXTERNFN gdbSendUserData()
{
#if GDBSTUB_ENABLE_UART2
	auto txbuf = user_uart == nullptr ? nullptr : user_uart->tx_buffer;
	if(txbuf == nullptr) {
		return 0;
	}

	size_t avail;
	void* data;
	while((avail = txbuf->getReadData(data)) != 0) {
		size_t charCount;
		unsigned used = uart_txfifo_count(GDB_UART);
		unsigned space = UART_TX_FIFO_SIZE - used - 1;
		if(gdb_state.attached) {
			// $Onn#CC is smallest packet, for a single character, but we want to avoid that as it's inefficient
			if(used >= 8) {
				break;
			}

			charCount = std::min((space - 3) / 2, avail);
			gdbWriteConsole(static_cast<const char*>(data), charCount);
		} else {
			charCount = gdb_uart_write(data, std::min(space, avail));
		}

		userDataSending = true;

		user_uart->tx_buffer->skipRead(charCount);
		if(charCount != avail) {
			break; // That's all for now
		}
	}

	return avail;
#else
	// UART2 disabled, no user data to send
	return 0;
#endif
}

void ATTR_GDBEXTERNFN gdbFlushUserData()
{
#if GDBSTUB_ENABLE_UART2
	while(gdbSendUserData() != 0) {
		wdt_feed();
		system_soft_wdt_feed();
	}
#endif
}

#if GDBSTUB_ENABLE_UART2

static void sendUserDataTask(uint32_t)
{
	sendUserDataQueued = false;

	if(gdbSendUserData() == 0) {
#if GDBSTUB_ENABLE_SYSCALL
		// When all data has been sent, see if there's a pending syscall to send
		gdbstub_syscall_execute();
#endif
	}
}

__forceinline void queueSendUserData()
{
	if(!sendUserDataQueued) {
		System.queueCallback(sendUserDataTask);
		sendUserDataQueued = true;
	}
}

/**
 * @brief Notify callback for user uart, called from uart driver
 */
static void userUartNotify(uart_t* uart, uart_notify_code_t code)
{
	switch(code) {
	case UART_NOTIFY_AFTER_OPEN:
		user_uart = uart;
		break;

	case UART_NOTIFY_BEFORE_CLOSE:
		user_uart = nullptr;
		break;

	case UART_NOTIFY_AFTER_WRITE: {
		/*
		 * Driver calls here from uart_write() after data has been written to tx buffer.
		 * Mostly we rely on task callback to transfer data from buffer to serial port,
		 * but we need to send some data directly if the buffer is full otherwise we'll
		 * loop indefinitely if UART_OPT_TXWAIT is set.
		 */
		if(uart->tx_buffer->isFull()) {
			gdbSendUserData();
		} else {
			queueSendUserData();
		}
		break;
	}

	case UART_NOTIFY_WAIT_TX: {
		gdbFlushUserData();
		break;
	}

	case UART_NOTIFY_BEFORE_READ:
		break;
	}
}
#endif

static void IRAM_ATTR doCtrlBreak()
{
	if(break_requests != 0) {
		break_requests = 0;
		gdbstub_break_internal(DBGFLAG_CTRL_BREAK);
	}
}

static void IRAM_ATTR gdb_uart_callback(uart_t* uart, uint32_t status)
{
#if GDBSTUB_ENABLE_UART2
	user_uart_status = status;

	// TX FIFO empty ?
	if(bitRead(status, UIFE)) {
		// Disable TX FIFO EMPTY interrupt to stop it recurring - re-enabled by uart_write()
		if(uart_txfifo_count(GDB_UART) == 0) {
			bitClear(USIE(GDB_UART), UIFE);
		}

		auto txbuf = user_uart == nullptr ? nullptr : user_uart->tx_buffer;
		if(txbuf != nullptr) {
			// More user data to send?
			if(!txbuf->isEmpty()) {
				// Yes - send it via task callback (because packetising code may not be in IRAM)
				queueSendUserData();
				bitClear(user_uart_status, UIFE);
			} else if(userDataSending) {
				// User data has now all been sent - UIFE will remain set
				userDataSending = false;
			} else {
				// No user data was in transit, so this event doesn't apply to user uart
				bitClear(user_uart_status, UIFE);
			}
		}
	}
#else
	bitClear(USIE(GDB_UART), UIFE);
#endif

	// RX FIFO Full or RX FIFO Timeout ?
	if(status & (_BV(UIFF) | _BV(UITO))) {
#if GDBSTUB_ENABLE_UART2
		auto rxbuf = user_uart == nullptr ? nullptr : user_uart->rx_buffer;
#endif

		while(uart_rxfifo_count(GDB_UART) != 0) {
			char c = USF(GDB_UART);
#if GDBSTUB_CTRLC_BREAK
			bool breakCheck = gdb_state.enabled;
#if GDBSTUB_CTRLC_BREAK == 1
			if(!gdb_state.attached) {
				breakCheck = false;
			}
#endif
			if(breakCheck && c == '\x03') {
				if(break_requests++ == 0) {
					// First attempt, break within a task callback
					System.queueCallback(TaskCallback(doCtrlBreak));
				} else if(break_requests == 3) {
					// Application failed to stop, break immediately
					doCtrlBreak();
				}
				continue;
			}
#endif

#if GDBSTUB_ENABLE_SYSCALL
			// If packet start detected whilst handling a system call, break into debugger to process it
			if(c == '$' && gdb_state.syscall == syscall_active) {
				gdbstub_break_internal(DBGFLAG_PACKET_STARTED);
				continue;
			}
#endif

#if GDBSTUB_ENABLE_UART2
			// When attached, nothing gets routed to UART2
			if(!gdb_state.attached && rxbuf != nullptr) {
				if(rxbuf->writeChar(c) == 0) {
					bitSet(user_uart_status, UIOF); // Overflow
				}
			}
#endif
		}

#if GDBSTUB_ENABLE_UART2
		if(rxbuf != nullptr) {
			if(rxbuf->isEmpty()) {
				// This event doesn't apply to user uart
				bitClear(user_uart_status, UIFF);
				bitClear(user_uart_status, UITO);
			} else if(rxbuf->getFreeSpace() > size_t(UART_RX_FIFO_SIZE) + user_uart->rx_headroom) {
				bitClear(user_uart_status, UIFF);
			}
		}
#endif
	}

#if GDBSTUB_ENABLE_UART2
	if(user_uart_status != 0 && user_uart != nullptr) {
		user_uart->status |= user_uart_status;
		if(user_uart->callback != nullptr) {
			user_uart->callback(user_uart, user_uart_status);
		}
	}
#endif
}

bool ATTR_GDBINIT gdb_uart_init()
{
	uart_set_debug(UART_NO);

	// Additional buffering not supported because interrupts are disabled when debugger halted
	uart_config cfg = {.uart_nr = GDB_UART,
					   .tx_pin = 1,
					   .mode = UART_FULL,
					   .options = _BV(UART_OPT_TXWAIT) | _BV(UART_OPT_CALLBACK_RAW),
					   .baudrate = SERIAL_BAUD_RATE,
					   .config = UART_8N1,
					   .rx_size = 0,
					   .tx_size = 0};
	gdb_uart = uart_init_ex(cfg);
	if(gdb_uart == nullptr) {
		return false;
	}
	uart_set_callback(gdb_uart, gdb_uart_callback, nullptr);

#if GDBSTUB_ENABLE_UART2
	// Virtualise user serial access via UART2
	uart_set_notify(UART2, userUartNotify);
	Serial.setPort(UART2);
#endif

#if GDBSTUB_ENABLE_DEBUG
	auto uart1 = uart_init(UART1, SERIAL_BAUD_RATE, UART_8N1, UART_TX_ONLY, 1, 0, 0);
	if(uart1 != nullptr) {
		uart_set_debug(UART1);
		m_setPuts(std::bind(&uart_write, uart1, _1, _2));
	}

	debug_i("\n\nGDB STUB RUNNING");
#endif

	return true;
}
