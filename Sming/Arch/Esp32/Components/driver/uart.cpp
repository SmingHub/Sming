/*
 uart.cpp - esp32 UART HAL
 */

#include <BitManipulations.h>

#include <driver/uart.h>
#include <driver/SerialBuffer.h>
#include <soc/uart_periph.h>
#include <driver/periph_ctrl.h>

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

// Determines whether to use APB or REF_TICK as clock source
constexpr bool uart_use_apb_clock{true};

static int s_uart_debug_nr = UART_NO;

// Keep track of interrupt enable state for each UART
static uint8_t isrMask;

struct smg_uart_hardware_t {
	volatile uart_dev_t& dev;
	const uart_signal_conn_t& conn;
	uint8_t tx_pin_default;
	uint8_t rx_pin_default;
};

constexpr smg_uart_hardware_t uartHardware[UART_COUNT] = {
	{UART0, uart_periph_signal[0], 1, 3},
	{UART1, uart_periph_signal[1], 10, 9},
	{UART2, uart_periph_signal[2], 17, 16},
};

// Keep a reference to all created UARTS
struct smg_uart_instance_t {
	smg_uart_t* uart;
	smg_uart_notify_callback_t callback;
	intr_handle_t handle;
};

static smg_uart_instance_t uartInstances[UART_COUNT];

// Get number of characters in receive FIFO
__forceinline static uint8_t uart_rxfifo_count(uint8_t nr)
{
	return uartHardware[nr].dev.status.rxfifo_cnt;
}

// Get number of characters in transmit FIFO
__forceinline static uint8_t uart_txfifo_count(uint8_t nr)
{
	return uartHardware[nr].dev.status.txfifo_cnt;
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

/** @brief Invoke a port callback, if one has been registered
 *  @param uart
 *  @param code
 */
static void notify(smg_uart_t* uart, smg_uart_notify_code_t code)
{
	auto callback = uartInstances[uart->uart_nr].callback;
	if(callback != nullptr) {
		callback(uart, code);
	}
}

__forceinline static bool uart_isr_enabled(uint8_t nr)
{
	return bitRead(isrMask, nr);
}

smg_uart_t* smg_uart_get_uart(uint8_t uart_nr)
{
	return (uart_nr < UART_COUNT) ? uartInstances[uart_nr].uart : nullptr;
}

uint8_t smg_uart_disable_interrupts()
{
	//	ETS_UART_INTR_DISABLE();
	return isrMask;
}

void smg_uart_restore_interrupts()
{
	if(isrMask != 0) {
		//		ETS_UART_INTR_ENABLE();
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
		auto& hw = uartHardware[uart->uart_nr];
		while(read < size && uart_rxfifo_count(uart->uart_nr) != 0) {
			buf[read++] = hw.dev.fifo.rw_byte;
		}

		// FIFO full may have been disabled if buffer overflowed, re-enabled it now
		decltype(uart_dev_t::int_clr) clr{};
		clr.rxfifo_full = true;
		clr.rxfifo_tout = true;
		clr.rxfifo_ovf = true;
		hw.dev.int_clr.val = clr.val;
		decltype(uart_dev_t::int_ena) ena{};
		ena.rxfifo_full = true;
		ena.rxfifo_tout = true;
		ena.rxfifo_ovf = true;
		hw.dev.int_ena.val = ena.val;
	}

	return read;
}

size_t smg_uart_rx_available(smg_uart_t* uart)
{
	if(!smg_uart_rx_enabled(uart)) {
		return 0;
	}

	smg_uart_disable_interrupts();

	size_t avail = is_physical(uart) ? uart_rxfifo_count(uart->uart_nr) : 0;

	if(uart->rx_buffer != nullptr) {
		avail += uart->rx_buffer->available();
	}

	smg_uart_restore_interrupts();

	return avail;
}

/** @brief UART interrupt service routine
 *  @note both UARTS share the same ISR, although UART1 only supports transmit
 */
static void IRAM_ATTR uart_isr(smg_uart_instance_t* inst)
{
	if(inst == nullptr || inst->uart == nullptr) {
		return;
	}

	auto uart = inst->uart;
	auto uart_nr = uart->uart_nr;
	auto& hw = uartHardware[uart_nr];

	decltype(uart_dev_t::int_st) usis;
	usis.val = hw.dev.int_st.val;

	// If status is clear there's no interrupt to service on this UART
	if(usis.val == 0) {
		return;
	}

	// Value to be passed to callback
	auto status = usis;

	// Deal with the event, unless we're in raw mode
	if(!bitRead(uart->options, UART_OPT_CALLBACK_RAW)) {
		// Rx FIFO full or timeout
		if(usis.rxfifo_full || usis.rxfifo_tout || usis.rxfifo_ovf) {
			size_t read = 0;

			// Read as much data as possible from the RX FIFO into buffer
			if(uart->rx_buffer != nullptr) {
				size_t avail = uart_rxfifo_count(uart_nr);
				size_t space = uart->rx_buffer->getFreeSpace();
				read = (avail <= space) ? avail : space;
				space -= read;
				while(read-- != 0) {
					uint8_t c = hw.dev.fifo.rw_byte;
					uart->rx_buffer->writeChar(c);
				}

				// Don't call back until buffer is (almost) full
				if(space > uart->rx_headroom) {
					status.rxfifo_full = false;
				}
			}

			/*
			 * If the FIFO is full and we didn't read any of the data then need to mask the interrupt out or it'll recur.
			 * The interrupt gets re-enabled by a call to uart_read() or uart_flush()
			 */
			if(usis.rxfifo_ovf) {
				hw.dev.int_ena.rxfifo_ovf = false;
			} else if(read == 0) {
				decltype(uart_dev_t::int_ena) ena{};
				ena.val = hw.dev.int_ena.val;
				ena.rxfifo_full = true;
				ena.rxfifo_tout = true;
				hw.dev.int_ena.val = ena.val;
			}
		}

		// Unless we replenish TX FIFO, disable after handling interrupt
		if(usis.txfifo_empty) {
			// Dump as much data as we can from buffer into the TX FIFO
			if(uart->tx_buffer != nullptr) {
				size_t space = uart_txfifo_free(uart_nr);
				size_t avail = uart->tx_buffer->available();
				size_t count = (avail <= space) ? avail : space;
				while(count-- != 0) {
					hw.dev.fifo.rw_byte = uart->tx_buffer->readChar();
				}
			}

			// If TX FIFO remains empty then we must disable TX FIFO EMPTY interrupt to stop it recurring.
			if(uart_txfifo_count(uart_nr) == 0) {
				// The interrupt gets re-enabled by uart_write()
				hw.dev.int_ena.txfifo_empty = false;
			} else {
				// We've topped up TX FIFO so defer callback until next time
				status.txfifo_empty = false;
			}
		}
	}

	// Keep a note of persistent flags - cleared via uart_get_status()
	uart->status |= status.val;

	if(status.val != 0 && uart->callback != nullptr) {
		uart->callback(uart, status.val);
	}

	// Final step is to clear status flags
	hw.dev.int_clr.val = usis.val;
}

void smg_uart_start_isr(smg_uart_t* uart)
{
	if(!is_physical(uart)) {
		return;
	}

	decltype(uart_dev_t::conf1) conf1{};
	decltype(uart_dev_t::int_ena) int_ena{};

	if(smg_uart_rx_enabled(uart)) {
		conf1.rxfifo_full_thrhd = 120;
		conf1.rx_tout_thrhd = 2;
		conf1.rx_tout_en = true;

		/*
		 * There is little benefit in generating interrupts on errors, instead these
		 * should be cleared at the start of a transaction and checked at the end.
		 * See uart_get_status().
		 */
		int_ena.rxfifo_full = true;
		int_ena.rxfifo_tout = true;
		int_ena.brk_det = true;
		int_ena.rxfifo_ovf = true;
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
		// conf1.txfifo_empty_thrhd = 0;
	}

	auto& hw = uartHardware[uart->uart_nr];
	hw.dev.conf1.val = conf1.val;
	hw.dev.int_clr.val = 0x0007ffff;
	hw.dev.int_ena.val = int_ena.val;

	smg_uart_disable_interrupts();
	auto& inst = uartInstances[uart->uart_nr];
	esp_intr_alloc(hw.conn.irq, ESP_INTR_FLAG_LOWMED, intr_handler_t(uart_isr), &inst, &inst.handle);
	smg_uart_restore_interrupts();
	bitSet(isrMask, uart->uart_nr);
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
				auto& hw = uartHardware[uart->uart_nr];
				while(written < size && !uart_txfifo_full(uart->uart_nr)) {
					hw.dev.fifo.rw_byte = buf[written++];
				}
				// Enable TX FIFO EMPTY interrupt
				hw.dev.int_clr.txfifo_empty = true;
				hw.dev.int_ena.txfifo_empty = true;
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

size_t smg_uart_tx_free(smg_uart_t* uart)
{
	if(!smg_uart_tx_enabled(uart)) {
		return 0;
	}

	smg_uart_disable_interrupts();

	size_t space = is_physical(uart) ? uart_txfifo_free(uart->uart_nr) : 0;
	if(uart->tx_buffer != nullptr) {
		space += uart->tx_buffer->getFreeSpace();
	}

	smg_uart_restore_interrupts();

	return space;
}

void smg_uart_wait_tx_empty(smg_uart_t* uart)
{
	if(!smg_uart_tx_enabled(uart)) {
		return;
	}

	notify(uart, UART_NOTIFY_WAIT_TX);

	if(uart->tx_buffer != nullptr) {
		while(!uart->tx_buffer->isEmpty()) {
			system_soft_wdt_feed();
		}
	}

	if(is_physical(uart)) {
		while(uart_txfifo_count(uart->uart_nr) != 0)
			system_soft_wdt_feed();
	}
}

void smg_uart_set_break(smg_uart_t* uart, bool state)
{
	uart = get_physical(uart);
	if(uart != nullptr) {
		auto& hw = uartHardware[uart->uart_nr];
		hw.dev.conf0.txd_brk = state;
	}
}

uint8_t smg_uart_get_status(smg_uart_t* uart)
{
	decltype(uart_dev_t::int_st) status{};
	if(uart != nullptr) {
		smg_uart_disable_interrupts();
		// Get break/overflow flags from actual uart (physical or otherwise)
		decltype(uart_dev_t::int_st) uart_status;
		uart_status.val = uart->status;
		status.brk_det = uart_status.brk_det;
		status.rxfifo_ovf = uart_status.rxfifo_ovf;
		uart->status = 0;
		// Read raw status register directly from real uart, masking out non-error bits
		uart = get_physical(uart);
		if(uart != nullptr) {
			auto& hw = uartHardware[uart->uart_nr];
			decltype(uart_dev_t::int_raw) int_raw;
			int_raw.val = hw.dev.int_raw.val;
			status.brk_det |= int_raw.brk_det;
			status.rxfifo_ovf |= int_raw.rxfifo_ovf;
			status.frm_err |= int_raw.frm_err;
			status.parity_err |= int_raw.parity_err;
			// Clear errors
			hw.dev.int_clr.val = status.val;
		}
		smg_uart_restore_interrupts();
	}

	return status.val;
}

void smg_uart_flush(smg_uart_t* uart, smg_uart_mode_t mode)
{
	if(uart == nullptr) {
		return;
	}

	bool flushRx = mode != UART_TX_ONLY && uart->mode != UART_TX_ONLY;
	bool flushTx = mode != UART_RX_ONLY && uart->mode != UART_RX_ONLY;

	smg_uart_disable_interrupts();
	if(flushRx && uart->rx_buffer != nullptr) {
		uart->rx_buffer->clear();
	}

	if(flushTx && uart->tx_buffer != nullptr) {
		uart->tx_buffer->clear();
	}

	if(is_physical(uart)) {
		auto& hw = uartHardware[uart->uart_nr];

		if(flushTx) {
			// Prevent TX FIFO EMPTY interrupts - don't need them until uart_write is called again
			hw.dev.int_ena.txfifo_empty = false;
			hw.dev.conf0.txfifo_rst = true;
			hw.dev.conf0.txfifo_rst = false;
		}

		// If receive overflow occurred then these interrupts will be masked
		if(flushRx) {
#if CONFIG_IDF_TARGET_ESP32
			// Hardware issue: we can not use `rxfifo_rst` to reset the hw rxfifo
			while(true) {
				auto fifo_cnt = hw.dev.status.rxfifo_cnt;
				decltype(uart_dev_t::mem_rx_status) stat;
				stat.val = hw.dev.mem_rx_status.val;
				if(fifo_cnt == 0 && (stat.rd_addr == stat.wr_addr)) {
					break;
				}

				(void)hw.dev.fifo.rw_byte;
			}
#else
			hw.dev.conf0.rxfifo_rst = true;
			hw.dev.conf0.rxfifo_rst = false;
#endif

			decltype(uart_dev_t::int_clr) int_clr;
			int_clr.val = 0x0007ffff;
			int_clr.txfifo_empty = false; // Leave this one
			hw.dev.int_clr.val = int_clr.val;

			decltype(uart_dev_t::int_ena) int_ena;
			int_ena.val = hw.dev.int_ena.val;
			int_ena.rxfifo_full = true;
			int_ena.rxfifo_tout = true;
			int_ena.rxfifo_ovf = true;
			hw.dev.int_ena.val = int_ena.val;
		}
	}

	smg_uart_restore_interrupts();
}

uint32_t smg_uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate)
{
	if(!is_physical(uart_nr) || baud_rate == 0) {
		return 0;
	}

	auto& hw = uartHardware[uart_nr];

	uint32_t sclk_freq = uart_use_apb_clock ? APB_CLK_FREQ : REF_CLK_FREQ;
	uint32_t clk_div = 16U * sclk_freq / baud_rate;
	// The baud-rate configuration register is divided into
	// an integer part and a fractional part.
	hw.dev.clk_div.div_int = clk_div / 16U;
	hw.dev.clk_div.div_frag = clk_div % 16U;
	// Configure the UART source clock.
	hw.dev.conf0.tick_ref_always_on = uart_use_apb_clock;

	// Return the actual baud rate in use
	return 16U * sclk_freq / clk_div;
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

smg_uart_t* smg_uart_init_ex(const smg_uart_config_t& cfg)
{
	// Already initialised?
	if(cfg.uart_nr >= UART_PHYSICAL_COUNT || uartInstances[cfg.uart_nr].uart != nullptr) {
		return nullptr;
	}

	auto uart = new smg_uart_t;
	if(uart == nullptr) {
		return nullptr;
	}

	auto& hw = uartHardware[cfg.uart_nr];

	memset(uart, 0, sizeof(smg_uart_t));
	uart->uart_nr = cfg.uart_nr;
	uart->mode = cfg.mode;
	uart->options = cfg.options;
	uart->rx_headroom = DEFAULT_RX_HEADROOM;

	int tx_pin = cfg.tx_pin;
	int rx_pin = cfg.rx_pin;

	auto rxBufferSize = cfg.rx_size;
	auto txBufferSize = cfg.tx_size;

	if(smg_uart_rx_enabled(uart)) {
		if(!realloc_buffer(uart->rx_buffer, rxBufferSize)) {
			delete uart;
			return nullptr;
		}

		// HardwareSerial default pin is 1 for all ports - check
		if(cfg.uart_nr != 0 && tx_pin == 1) {
			tx_pin = hw.tx_pin_default;
		} else {
			tx_pin = (tx_pin == UART_PIN_DEFAULT) ? hw.tx_pin_default : cfg.tx_pin;
		}
	} else {
		tx_pin = UART_PIN_NO_CHANGE;
	}

	if(smg_uart_tx_enabled(uart)) {
		if(!realloc_buffer(uart->tx_buffer, txBufferSize)) {
			delete uart->rx_buffer;
			delete uart;
			return nullptr;
		}

		rx_pin = (cfg.rx_pin == UART_PIN_DEFAULT) ? hw.rx_pin_default : cfg.rx_pin;
	} else {
		rx_pin = UART_PIN_NO_CHANGE;
	}

	// OK, buffers allocated so setup hardware
	smg_uart_detach(cfg.uart_nr);
	smg_uart_set_pins(uart, tx_pin, rx_pin);

	periph_module_reset(hw.conn.module);
	periph_module_enable(hw.conn.module);

	// Bottom 8 bits identical to esp8266
	hw.dev.conf0.val = (hw.dev.conf0.val & 0xFFFFFF00) | cfg.config;

	smg_uart_set_baudrate(uart, cfg.baudrate);
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

	auto& hw = uartHardware[uart->uart_nr];
	periph_module_disable(hw.conn.module);

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
	/*
	if(uart == nullptr) {
		return;
	}

	switch(uart->uart_nr) {
	case UART0:
		uart0_pin_restore(uart->tx_pin);
		uart0_pin_restore(uart->rx_pin);

		if(uart->tx_pin == 1 || uart->tx_pin == 2 || uart->rx_pin == 3) {
			if(smg_uart_tx_enabled(uart)) {
				uart->tx_pin = 15;
			}

			if(smg_uart_rx_enabled(uart)) {
				uart->rx_pin = 13;
			}

			SET_PERI_REG_MASK(UART_SWAP_REG, UART_SWAP0);
		} else {
			if(smg_uart_tx_enabled(uart)) {
				uart->tx_pin = (tx_pin == 2) ? 2 : 1;
			}

			if(smg_uart_rx_enabled(uart)) {
				uart->rx_pin = 3;
			}

			CLEAR_PERI_REG_MASK(UART_SWAP_REG, UART_SWAP0);
		}

		uart0_pin_select(uart->tx_pin);
		uart0_pin_select(uart->rx_pin);
		break;

	case UART1:
		// Currently no swap possible! See GPIO pins used by UART
		break;

	default:
		break;
	}
*/
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

	if(tx_pin != UART_PIN_NO_CHANGE && !GPIO_IS_VALID_OUTPUT_GPIO(tx_pin)) {
		return false;
	}

	if(rx_pin != UART_PIN_NO_CHANGE && !GPIO_IS_VALID_OUTPUT_GPIO(rx_pin)) {
		return false;
	}

	auto& hw = uartHardware[uart->uart_nr];

	if(tx_pin != UART_PIN_NO_CHANGE) {
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[tx_pin], PIN_FUNC_GPIO);
		gpio_set_level(gpio_num_t(tx_pin), true);
		gpio_matrix_out(tx_pin, hw.conn.tx_sig, false, false);
		uart->tx_pin = tx_pin;
	}

	if(rx_pin != UART_PIN_NO_CHANGE) {
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[rx_pin], PIN_FUNC_GPIO);
		gpio_set_pull_mode(gpio_num_t(rx_pin), GPIO_PULLUP_ONLY);
		gpio_set_direction(gpio_num_t(rx_pin), GPIO_MODE_INPUT);
		gpio_matrix_in(rx_pin, hw.conn.rx_sig, false);
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
	ets_install_putc1(smg_uart_debug_putc);
	ets_install_putc2(nullptr);
}

int smg_uart_get_debug()
{
	return s_uart_debug_nr;
}

void smg_uart_detach(int uart_nr)
{
	if(!is_physical(uart_nr)) {
		return;
	}

	smg_uart_disable_interrupts();

	if(bitRead(isrMask, uart_nr)) {
		auto& inst = uartInstances[uart_nr];
		esp_intr_free(inst.handle);
		inst.handle = nullptr;
		bitClear(isrMask, uart_nr);
	}

	auto& hw = uartHardware[uart_nr];
	hw.dev.conf1.val = 0;
	hw.dev.int_clr.val = 0x0007ffff;
	hw.dev.int_ena.val = 0;
	smg_uart_restore_interrupts();
}

void smg_uart_detach_all()
{
	smg_uart_disable_interrupts();
	for(unsigned uart_nr = 0; uart_nr < UART_PHYSICAL_COUNT; ++uart_nr) {
		if(bitRead(isrMask, uart_nr)) {
			auto& inst = uartInstances[uart_nr];
			esp_intr_free(inst.handle);
			inst.handle = nullptr;
		}
		auto& hw = uartHardware[uart_nr];
		hw.dev.conf1.val = 0;
		hw.dev.int_clr.val = 0x0007ffff;
		hw.dev.int_ena.val = 0;
	}
	isrMask = 0;
}
