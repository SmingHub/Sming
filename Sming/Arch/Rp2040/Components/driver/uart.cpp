/*
 uart.cpp - RP2040 UART driver
 */

#include <BitManipulations.h>

#include <driver/uart.h>
#include <driver/SerialBuffer.h>
#include <pico.h>
#include <hardware/structs/uart.h>
#include <hardware/clocks.h>
#include <hardware/irq.h>
#include <hardware/resets.h>
#include <hardware/gpio.h>

namespace
{
using uart_dev_t = uart_hw_t;

int s_uart_debug_nr = UART_NO;

// Keep track of interrupt enable state for each UART
uint8_t isrMask;

struct smg_uart_pins_t {
	uint8_t tx;
	uint8_t rx;
};

#define UART0_PIN_DEFAULT PICO_DEFAULT_UART_TX_PIN, PICO_DEFAULT_UART_RX_PIN
#define UART1_PIN_DEFAULT 4, 5

constexpr smg_uart_pins_t defaultPins[UART_COUNT] = {
	{UART0_PIN_DEFAULT},
	{UART1_PIN_DEFAULT},
};

uart_dev_t* IRAM_ATTR getDevice(uint8_t uart_nr)
{
	if(uart_nr == 0) {
		return uart0_hw;
	}
	if(uart_nr == 1) {
		return uart1_hw;
	}
	assert(false);
	return nullptr;
}

// Keep a reference to all created UARTS
struct smg_uart_instance_t {
	smg_uart_t* uart;
	smg_uart_notify_callback_t callback;
};

smg_uart_instance_t uartInstances[UART_COUNT];

// Return true if transmit FIFO is full
__forceinline bool uart_txfifo_full(uart_dev_t* dev)
{
	return !!(dev->fr & UART_UARTFR_TXFF_BITS);
}

// Return true if transmit FIFO is empty
__forceinline bool uart_txfifo_empty(uart_dev_t* dev)
{
	return !!(dev->fr & UART_UARTFR_TXFE_BITS);
}

// Return true if receive FIFO is empty
__forceinline bool uart_rxfifo_empty(uart_dev_t* dev)
{
	return !!(dev->fr & UART_UARTFR_RXFE_BITS);
}

/** @brief Invoke a port callback, if one has been registered
 *  @param uart
 *  @param code
 */
void notify(smg_uart_t* uart, smg_uart_notify_code_t code)
{
	auto callback = uartInstances[uart->uart_nr].callback;
	if(callback != nullptr) {
		callback(uart, code);
	}
}

__forceinline void uart_disable_isr(uint8_t nr)
{
	irq_set_enabled(UART0_IRQ + nr, false);
}

__forceinline void uart_enable_isr(uint8_t nr)
{
	irq_set_enabled(UART0_IRQ + nr, true);
}

bool realloc_buffer(SerialBuffer*& buffer, size_t new_size)
{
	if(buffer != nullptr) {
		if(new_size == 0) {
			delete buffer;
			buffer = nullptr;
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

void IRAM_ATTR handleInterrupt(smg_uart_t* uart, uart_dev_t* dev)
{
	auto mis = dev->mis;

	// If status is clear there's no interrupt to service on this UART
	if(mis == 0) {
		return;
	}

	// Value to be passed to callback
	auto user_is = mis;

	// Deal with the event, unless we're in raw mode
	if(!bitRead(uart->options, UART_OPT_CALLBACK_RAW)) {
		// Rx FIFO full, timeout or receive overflow
		auto rxfifo_full = mis & UART_UARTMIS_RXMIS_BITS;
		auto rxfifo_tout = mis & UART_UARTMIS_RTMIS_BITS;
		auto rxfifo_ovf = mis & UART_UARTMIS_OEMIS_BITS;
		if(rxfifo_full || rxfifo_tout || rxfifo_ovf) {
			bool read{false};

			// Read as much data as possible from the RX FIFO into buffer
			if(uart->rx_buffer != nullptr) {
				size_t space = uart->rx_buffer->getFreeSpace();
				while(space != 0 && !uart_rxfifo_empty(dev)) {
					uint8_t c = dev->dr;
					uart->rx_buffer->writeChar(c);
					--space;
					read = true;
				}

				// Don't call back until buffer is (almost) full
				if(space > uart->rx_headroom) {
					user_is &= ~UART_UARTMIS_RXMIS_BITS;
				}
			}

			/*
			 * If the FIFO is full and we didn't read any of the data then need to mask the interrupt out or it'll recur.
			 * The interrupt gets re-enabled by a call to uart_read() or uart_flush()
			 */
			if(rxfifo_ovf) {
				hw_clear_bits(&dev->imsc, UART_UARTIMSC_OEIM_BITS);
			} else if(read == 0) {
				hw_set_bits(&dev->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);
			}
		}

		// Unless we replenish TX FIFO, disable after handling interrupt
		auto txfifo_empty = mis & UART_UARTMIS_TXMIS_BITS;
		if(txfifo_empty) {
			// Dump as much data as we can from buffer into the TX FIFO
			if(uart->tx_buffer != nullptr) {
				size_t avail = uart->tx_buffer->available();
				while(avail-- && !uart_txfifo_full(dev)) {
					uint8_t c = uart->tx_buffer->readChar();
					dev->dr = c;
				}
			}

			// If TX FIFO remains empty then we must disable TX FIFO EMPTY interrupt to stop it recurring.
			if(uart_txfifo_empty(dev)) {
				// The interrupt gets re-enabled by uart_write()
				hw_clear_bits(&dev->imsc, UART_UARTIMSC_TXIM_BITS);
			} else {
				// We've topped up TX FIFO so defer callback until next time
				user_is &= ~UART_UARTMIS_TXMIS_BITS;
			}
		}
	}

	// Translate interrupt status flag bits into API values
	uint32_t status{0};
	if(user_is & UART_UARTMIS_RXMIS_BITS) {
		status |= UART_STATUS_RXFIFO_FULL;
	}
	if(user_is & UART_UARTMIS_RTMIS_BITS) {
		status |= UART_STATUS_RXFIFO_TOUT;
	}
	if(user_is & UART_UARTMIS_OEMIS_BITS) {
		status |= UART_STATUS_RXFIFO_OVF;
	}
	if(user_is & UART_UARTMIS_TXMIS_BITS) {
		status |= UART_STATUS_TXFIFO_EMPTY;
	}
	if(user_is & UART_UARTMIS_BEMIS_BITS) {
		status |= UART_STATUS_BRK_DET;
	}
	if(user_is & UART_UARTMIS_FEMIS_BITS) {
		status |= UART_STATUS_FRM_ERR;
	}

	// Keep a note of persistent flags - cleared via uart_get_status()
	uart->status |= status;

	if(status != 0 && uart->callback != nullptr) {
		uart->callback(uart, status);
	}

	// Final step is to clear status flags
	dev->icr = mis;
}

void IRAM_ATTR uart0_isr()
{
	handleInterrupt(uartInstances[0].uart, uart0_hw);
}

void IRAM_ATTR uart1_isr()
{
	handleInterrupt(uartInstances[1].uart, uart1_hw);
}

} // namespace

smg_uart_t* smg_uart_get_uart(uint8_t uart_nr)
{
	return (uart_nr < UART_COUNT) ? uartInstances[uart_nr].uart : nullptr;
}

uint8_t smg_uart_disable_interrupts()
{
	if(isrMask & BIT(0)) {
		irq_set_enabled(UART0_IRQ, false);
	}
	if(isrMask & BIT(1)) {
		irq_set_enabled(UART1_IRQ, false);
	}
	return isrMask;
}

void smg_uart_restore_interrupts()
{
	if(isrMask & BIT(0)) {
		irq_set_enabled(UART0_IRQ, true);
	}
	if(isrMask & BIT(1)) {
		irq_set_enabled(UART1_IRQ, true);
	}
}

bool smg_uart_set_notify(unsigned uart_nr, smg_uart_notify_callback_t callback)
{
	if(uart_nr >= UART_COUNT) {
		return false;
	}

	uartInstances[uart_nr].callback = callback;
	return true;
}

void smg_uart_set_callback(smg_uart_t* uart, smg_uart_callback_t callback, void* param)
{
	if(uart != nullptr) {
		uart->callback = nullptr; // In case interrupt fires between setting param and callback
		uart->param = param;
		uart->callback = callback;
	}
}

size_t smg_uart_resize_rx_buffer(smg_uart_t* uart, size_t new_size)
{
	if(smg_uart_rx_enabled(uart)) {
		uart_disable_isr(uart->uart_nr);
		realloc_buffer(uart->rx_buffer, new_size);
		uart_enable_isr(uart->uart_nr);
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
		uart_disable_isr(uart->uart_nr);
		realloc_buffer(uart->tx_buffer, new_size);
		uart_enable_isr(uart->uart_nr);
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
	auto dev = getDevice(uart->uart_nr);
	while(read < size && !uart_rxfifo_empty(dev)) {
		uint8_t c = dev->dr;
		buf[read++] = c;
	}

	// FIFO full may have been disabled if buffer overflowed, re-enabled it now
	dev->icr = UART_UARTMIS_RXMIS_BITS | UART_UARTMIS_RTMIS_BITS | UART_UARTMIS_OEMIS_BITS;
	hw_set_bits(&dev->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS | UART_UARTIMSC_OEIM_BITS);

	return read;
}

size_t smg_uart_rx_available(smg_uart_t* uart)
{
	return uart->rx_buffer ? uart->rx_buffer->available() : 0;
}

void smg_uart_start_isr(smg_uart_t* uart)
{
	uint32_t int_ena{0};
	uint32_t fifo_level_select{0};

	auto dev = getDevice(uart->uart_nr);

	if(smg_uart_rx_enabled(uart)) {
		// Trigger at >= 7/8 full
		fifo_level_select |= 5 << UART_UARTIFLS_RXIFLSEL_LSB;

		/*
		 * There is little benefit in generating interrupts on errors, instead these
		 * should be cleared at the start of a transaction and checked at the end.
		 * See uart_get_status().
		 */
		int_ena |= UART_UARTIMSC_RXIM_BITS   // rxfifo_full
				   | UART_UARTIMSC_RTIM_BITS // rxfifo_tout
				   | UART_UARTIMSC_OEIM_BITS // rxfifo_ovf
				   | UART_UARTIMSC_BEIM_BITS // brk_det
				   | UART_UARTIMSC_PEIM_BITS // parity error
				   | UART_UARTIMSC_FEIM_BITS // framing error
			;
	}

	if(smg_uart_tx_enabled(uart)) {
		/*
		 * We can interrupt when TX FIFO is empty; at 1Mbit that gives us 800 CPU
		 * cycles before the last character has actually gone over the wire. Even if
		 * a gap occurs it is unlike to cause any problems. It also makes the callback
		 * more useful, for example if using it for RS485 we'd then want to reverse
		 * transfer direction and begin waiting for a response.
		 */

		// TX FIFO empty interrupt only gets enabled via uart_write function()

		// Trigger at <= 1/8 full
		fifo_level_select |= 0 << UART_UARTIFLS_TXIFLSEL_LSB;
	}

	dev->ifls = fifo_level_select;

	// Clear and enable required UART interrupts
	dev->icr = UART_UARTICR_BITS;
	dev->imsc = int_ena;

	// Enable interrupt handler
	auto irq = UART0_IRQ + uart->uart_nr;
	irq_set_exclusive_handler(irq, (uart->uart_nr == 0) ? uart0_isr : uart1_isr);
	irq_set_enabled(irq, true);
	bitSet(isrMask, uart->uart_nr);
}

size_t smg_uart_write(smg_uart_t* uart, const void* buffer, size_t size)
{
	if(!smg_uart_tx_enabled(uart) || buffer == nullptr || size == 0) {
		return 0;
	}

	size_t written = 0;

	auto buf = static_cast<const uint8_t*>(buffer);

	while(written < size) {
		// If TX buffer not in use or it's empty then write directly to hardware FIFO
		if(uart->tx_buffer == nullptr || uart->tx_buffer->isEmpty()) {
			auto dev = getDevice(uart->uart_nr);
			while(written < size && !uart_txfifo_full(dev)) {
				dev->dr = buf[written++];
			}

			// Enable TX FIFO EMPTY interrupt
			dev->icr = UART_UARTMIS_TXMIS_BITS;
			hw_set_bits(&dev->imsc, UART_UARTIMSC_TXIM_BITS);
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

size_t smg_uart_tx_free(smg_uart_t* uart)
{
	return uart->tx_buffer ? uart->tx_buffer->getFreeSpace() : 0;
}

void smg_uart_wait_tx_empty(smg_uart_t* uart)
{
	if(!smg_uart_tx_enabled(uart)) {
		return;
	}

	notify(uart, UART_NOTIFY_WAIT_TX);

	if(uart->tx_buffer != nullptr) {
		while(!uart->tx_buffer->isEmpty()) {
		}
	}

	auto dev = getDevice(uart->uart_nr);
	while((dev->fr & UART_UARTFR_TXFE_BITS) != 0) {
	}
}

void smg_uart_set_break(smg_uart_t* uart, bool state)
{
	if(uart == nullptr) {
		return;
	}

	auto dev = getDevice(uart->uart_nr);
	if(state) {
		hw_set_bits(&dev->lcr_h, UART_UARTLCR_H_BRK_BITS);
	} else {
		hw_clear_bits(&dev->lcr_h, UART_UARTLCR_H_BRK_BITS);
	}
}

uint8_t smg_uart_get_status(smg_uart_t* uart)
{
	if(uart == nullptr) {
		return 0;
	}

	uart_disable_isr(uart->uart_nr);
	auto dev = getDevice(uart->uart_nr);
	// Get and clear break/overflow flags from uart
	auto rsr = dev->rsr & (UART_UARTRSR_BE_BITS | UART_UARTRSR_OE_BITS | UART_UARTRSR_PE_BITS);
	dev->rsr = rsr; // Clear errors
	// Read raw status directly from uart, masking out non-error bits
	auto raw = dev->ris & (UART_UARTRIS_OERIS_BITS | UART_UARTRIS_BERIS_BITS | UART_UARTRIS_FERIS_BITS);
	dev->icr = raw; // clear errors
	// Fetch latched status and clear it
	auto status = uart->status;
	uart->status = 0;
	uart_enable_isr(uart->uart_nr);

	// Now translate hardware flags into API values
	if(rsr & UART_UARTRSR_BE_BITS) {
		status |= UART_STATUS_BRK_DET;
	}
	if(rsr & UART_UARTRSR_OE_BITS) {
		status |= UART_STATUS_RXFIFO_OVF;
	}
	if(rsr & UART_UARTRSR_PE_BITS) {
		status |= UART_STATUS_PARITY_ERR;
	}
	if(raw & UART_UARTRIS_OERIS_BITS) {
		status |= UART_STATUS_RXFIFO_OVF;
	}
	if(raw & UART_UARTRIS_BERIS_BITS) {
		status |= UART_STATUS_BRK_DET;
	}
	if(raw & UART_UARTRIS_FERIS_BITS) {
		status |= UART_STATUS_FRM_ERR;
	}

	return status;
}

void smg_uart_flush(smg_uart_t* uart, smg_uart_mode_t mode)
{
	if(uart == nullptr) {
		return;
	}

	bool flushRx = mode == UART_FULL || mode == UART_RX_ONLY;
	bool flushTx = mode == UART_FULL || mode == UART_TX_ONLY;

	uart_disable_isr(uart->uart_nr);
	if(flushRx && uart->rx_buffer != nullptr) {
		uart->rx_buffer->clear();
	}

	if(flushTx && uart->tx_buffer != nullptr) {
		uart->tx_buffer->clear();
	}

	auto dev = getDevice(uart->uart_nr);

	if(flushTx) {
		// Prevent TX FIFO EMPTY interrupts - don't need them until uart_write is called again
		hw_clear_bits(&dev->imsc, UART_UARTIMSC_TXIM_BITS);
		while(!uart_txfifo_empty(dev)) {
			//
		}
	}

	// If receive overflow occurred then these interrupts will be masked
	if(flushRx) {
		while(!uart_rxfifo_empty(dev)) {
			uint8_t c = dev->dr;
			(void)c;
		}

		// Clear all receive flag bits
		dev->icr = UART_UARTICR_RXIC_BITS   // rxfifo_full
				   | UART_UARTICR_RTIC_BITS // rxfifo_tout
				   | UART_UARTICR_OEIC_BITS // rxfifo_ovf
			;

		hw_set_bits(&dev->imsc, UART_UARTIMSC_RXIM_BITS		  // rxfifo_full
									| UART_UARTIMSC_RTIM_BITS // rxfifo_tout
									| UART_UARTIMSC_OEIM_BITS // rxfifo_ovf
		);
	}

	uart_enable_isr(uart->uart_nr);
}

uint32_t smg_uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate)
{
	if(baud_rate == 0) {
		return 0;
	}

	auto clock_hz = clock_get_hz(clk_peri);
	uint32_t baud_rate_div = 8 * clock_hz / baud_rate;
	uint32_t baud_ibrd = baud_rate_div >> 7;
	uint32_t baud_fbrd{0};

	if(baud_ibrd == 0) {
		baud_ibrd = 1;
	} else if(baud_ibrd >= 0xffff) {
		baud_ibrd = 0xffff;
	} else {
		baud_fbrd = ((baud_rate_div & 0x7f) + 1) / 2;
	}

	// Load PL011's baud divisor registers
	auto dev = getDevice(uart_nr);
	dev->ibrd = baud_ibrd;
	dev->fbrd = baud_fbrd;

	// PL011 needs a (dummy) line control register write to latch in the
	// divisors. We don't want to actually change LCR contents here.
	hw_set_bits(&dev->lcr_h, 0);

	// Return the actual baud rate in use
	return 4 * clock_hz / ((64 * baud_ibrd) + baud_fbrd);
}

uint32_t smg_uart_set_baudrate(smg_uart_t* uart, uint32_t baud_rate)
{
	baud_rate = smg_uart_set_baudrate_reg(uart->uart_nr, baud_rate);
	// Store the actual baud rate in use
	uart->baud_rate = baud_rate;
	return baud_rate;
}

uint32_t smg_uart_get_baudrate(smg_uart_t* uart)
{
	return uart ? uart->baud_rate : 0;
}

smg_uart_t* smg_uart_init_ex(const smg_uart_config_t& cfg)
{
	// Already initialised?
	if(cfg.uart_nr >= UART_COUNT || uartInstances[cfg.uart_nr].uart != nullptr) {
		return nullptr;
	}

	auto uart = new smg_uart_t{};
	if(uart == nullptr) {
		return nullptr;
	}

	uart->uart_nr = cfg.uart_nr;
	uart->mode = cfg.mode;
	uart->options = cfg.options;
	uart->tx_pin = uart->rx_pin = UART_PIN_DEFAULT;
	uart->rx_headroom = 16;

	int tx_pin = cfg.tx_pin;
	int rx_pin = cfg.rx_pin;

	auto rxBufferSize = cfg.rx_size;
	auto txBufferSize = cfg.tx_size;

	if(smg_uart_rx_enabled(uart)) {
		if(!realloc_buffer(uart->rx_buffer, rxBufferSize)) {
			delete uart;
			return nullptr;
		}
		rx_pin = (cfg.rx_pin == UART_PIN_DEFAULT) ? defaultPins[cfg.uart_nr].rx : cfg.rx_pin;
	} else {
		rx_pin = UART_PIN_NO_CHANGE;
	}

	if(smg_uart_tx_enabled(uart)) {
		if(!realloc_buffer(uart->tx_buffer, txBufferSize)) {
			delete uart->rx_buffer;
			delete uart;
			return nullptr;
		}
		tx_pin = (tx_pin == UART_PIN_DEFAULT) ? defaultPins[cfg.uart_nr].tx : cfg.tx_pin;
	} else {
		tx_pin = UART_PIN_NO_CHANGE;
	}

	// OK, buffers allocated so setup hardware
	auto reset_bits = (uart->uart_nr == 1) ? RESETS_RESET_UART1_BITS : RESETS_RESET_UART0_BITS;
	reset_block(reset_bits);
	unreset_block_wait(reset_bits);

	smg_uart_detach(cfg.uart_nr);
	smg_uart_set_baudrate(uart, cfg.baudrate);

	auto dev = getDevice(cfg.uart_nr);

	// Setup line control register
	smg_uart_config_format_t fmt;
	fmt.val = cfg.config;
	uint32_t lcr{0};
	lcr |= fmt.bits << UART_UARTLCR_H_WLEN_LSB; // data bits
	if(fmt.stop_bits != UART_NB_STOP_BIT_1) {   // stop bits
		lcr |= UART_UARTLCR_H_STP2_BITS;
	}
	lcr |= fmt.parity << UART_UARTLCR_H_PEN_LSB; // parity
	lcr |= UART_UARTLCR_H_FEN_BITS;				 // Enable FIFOs
	dev->lcr_h = lcr;

	// Enable the UART
	if(uart->mode == UART_TX_ONLY) {
		dev->cr = UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS;
	} else if(uart->mode == UART_RX_ONLY) {
		dev->cr = UART_UARTCR_UARTEN_BITS | UART_UARTCR_RXE_BITS;
	} else {
		dev->cr = UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS | UART_UARTCR_RXE_BITS;
	}

	// Always enable DREQ signals -- no harm in this if DMA is not listening
	dev->dmacr = UART_UARTDMACR_TXDMAE_BITS | UART_UARTDMACR_RXDMAE_BITS;

	smg_uart_set_pins(uart, tx_pin, rx_pin);

	smg_uart_flush(uart);
	uartInstances[cfg.uart_nr].uart = uart;
	smg_uart_start_isr(uart);

	notify(uart, UART_NOTIFY_AFTER_OPEN);

	return uart;
}

void smg_uart_uninit(smg_uart_t* uart)
{
	if(uart == nullptr) {
		return;
	}

	notify(uart, UART_NOTIFY_BEFORE_CLOSE);

	smg_uart_stop_isr(uart);
	// If debug output being sent to this UART, disable it
	if(uart->uart_nr == s_uart_debug_nr) {
		smg_uart_set_debug(UART_NO);
	}

	auto reset_bits = (uart->uart_nr == 1) ? RESETS_RESET_UART1_BITS : RESETS_RESET_UART0_BITS;
	reset_block(reset_bits);

	uartInstances[uart->uart_nr].uart = nullptr;
	delete uart->rx_buffer;
	delete uart->tx_buffer;
	delete uart;
}

smg_uart_t* smg_uart_init(uint8_t uart_nr, uint32_t baudrate, uint32_t config, smg_uart_mode_t mode, uint8_t tx_pin,
						  size_t rx_size, size_t tx_size)
{
	smg_uart_config_t cfg = {.uart_nr = uart_nr,
							 .tx_pin = tx_pin,
							 .rx_pin = UART_PIN_DEFAULT,
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
	// Not implemented
}

bool smg_uart_set_tx(smg_uart_t* uart, int tx_pin)
{
	return uart == nullptr ? false : smg_uart_set_pins(uart, tx_pin, -1);
}

bool smg_uart_set_pins(smg_uart_t* uart, int tx_pin, int rx_pin)
{
	if(uart == nullptr) {
		return false;
	}

	// Valid GPIO for UART TX function
	auto is_tx_pin = [&](uint8_t pin) -> bool {
		const uint8_t uart_tx_pins[2][4] = {
			{0, 12, 16, 28},
			{4, 8, 20, 24},
		};
		auto& txpins = uart_tx_pins[uart->uart_nr];
		return pin == txpins[0] || pin == txpins[1] || pin == txpins[2] || pin == txpins[3];
	};
	// UART pins are in groups, RX pins follow TX
	auto is_rx_pin = [&](uint8_t pin) -> bool { return is_tx_pin(pin - 1); };

	if(tx_pin != UART_PIN_NO_CHANGE && !is_tx_pin(tx_pin)) {
		return false;
	}
	if(rx_pin != UART_PIN_NO_CHANGE && !is_rx_pin(rx_pin)) {
		return false;
	}

	if(tx_pin != UART_PIN_NO_CHANGE) {
		if(uart->tx_pin != UART_PIN_DEFAULT) {
			gpio_set_function(uart->tx_pin, GPIO_FUNC_NULL);
		}
		gpio_set_function(tx_pin, GPIO_FUNC_UART);
		uart->tx_pin = tx_pin;
	}

	if(rx_pin != UART_PIN_NO_CHANGE) {
		if(uart->rx_pin != UART_PIN_DEFAULT) {
			gpio_set_function(uart->rx_pin, GPIO_FUNC_NULL);
		}
		gpio_set_function(rx_pin, GPIO_FUNC_UART);
		uart->rx_pin = rx_pin;
	}

	return true;
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
	s_uart_debug_nr = uart_nr;
	system_set_os_print(uart_nr >= 0);
}

int smg_uart_get_debug()
{
	return s_uart_debug_nr;
}

void smg_uart_detach(int uart_nr)
{
	if(!bitRead(isrMask, uart_nr)) {
		return;
	}

	uart_disable_isr(uart_nr);
	auto dev = getDevice(uart_nr);
	dev->imsc = 0;

	bitClear(isrMask, uart_nr);
}

void smg_uart_detach_all()
{
	smg_uart_detach(0);
	smg_uart_detach(1);
}
