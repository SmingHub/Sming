/*
 uart.cpp - esp32 UART HAL
 */

// These conflict with enumerated types defined in IDF
#define UART_PARITY_NONE IDF_UART_PARITY_NONE
#define UART_PARITY_EVEN IDF_UART_PARITY_EVEN
#define UART_PARITY_ODD IDF_UART_PARITY_ODD
#include <hal/uart_ll.h>
#undef UART_PARITY_NONE
#undef UART_PARITY_EVEN
#undef UART_PARITY_ODD

#include <driver/uart.h>
#include <driver/SerialBuffer.h>
#include <driver/periph_ctrl.h>
#include <soc/uart_channel.h>
#include <BitManipulations.h>
#include <Data/Range.h>
#include <esp_systemapi.h>
#include <hal/gpio_ll.h>

namespace
{
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

int s_uart_debug_nr = UART_NO;

// Keep track of interrupt enable state for each UART
uint8_t isrMask;

struct smg_uart_pins_t {
	uint8_t tx;
	uint8_t rx;
};

#ifdef SOC_ESP32
#define UART0_PIN_DEFAULT UART_NUM_0_TXD_DIRECT_GPIO_NUM, UART_NUM_0_RXD_DIRECT_GPIO_NUM
#define UART1_PIN_DEFAULT 18, 19 // Defaults conflict with flash
#define UART2_PIN_DEFAULT UART_NUM_2_TXD_DIRECT_GPIO_NUM, UART_NUM_2_RXD_DIRECT_GPIO_NUM
#elif defined(SOC_ESP32C3)
#define UART0_PIN_DEFAULT 21, 20
#define UART1_PIN_DEFAULT 10, 9
#elif defined(SOC_ESP32S2)
#define UART0_PIN_DEFAULT 43, 44
#define UART1_PIN_DEFAULT UART_NUM_1_TXD_DIRECT_GPIO_NUM, UART_NUM_1_RXD_DIRECT_GPIO_NUM
#elif defined(SOC_ESP32S3)
#define UART0_PIN_DEFAULT 43, 44
#define UART1_PIN_DEFAULT UART_NUM_1_TXD_DIRECT_GPIO_NUM, UART_NUM_1_RXD_DIRECT_GPIO_NUM
#define UART2_PIN_DEFAULT 17, 16
#else
static_assert(false, "Must define default UART pins for selected ESP_VARIANT");
#endif

constexpr smg_uart_pins_t defaultPins[UART_COUNT] = {
	{UART0_PIN_DEFAULT},
	{UART1_PIN_DEFAULT},
#if UART_COUNT > 2
	{UART2_PIN_DEFAULT},
#endif
};

uart_dev_t* IRAM_ATTR getDevice(uint8_t uart_nr)
{
	if(uart_nr == 0) {
		return &UART0;
	}
	if(uart_nr == 1) {
		return &UART1;
	}
#if UART_COUNT > 2
	if(uart_nr == 2) {
		return &UART2;
	}
#endif
	assert(false);
	return nullptr;
}

// Keep a reference to all created UARTS
struct smg_uart_instance_t {
	smg_uart_t* uart;
	smg_uart_notify_callback_t callback;
	intr_handle_t handle;
};

smg_uart_instance_t uartInstances[UART_COUNT];

// Get number of characters in transmit FIFO
__forceinline size_t uart_txfifo_count(uart_dev_t* dev)
{
	return dev->status.txfifo_cnt;
}

// Get available free characters in transmit FIFO
__forceinline size_t uart_txfifo_free(uart_dev_t* dev)
{
	return UART_TX_FIFO_SIZE - uart_txfifo_count(dev) - 1;
}

// Return true if transmit FIFO is full
__forceinline bool uart_txfifo_full(uart_dev_t* dev)
{
	return uart_txfifo_count(dev) >= (UART_TX_FIFO_SIZE - 1);
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

/** @brief Determine if the given uart is a real uart or a virtual one
 */
__forceinline bool is_physical(int uart_nr)
{
	return (uart_nr >= 0) && (uart_nr < UART_PHYSICAL_COUNT);
}

__forceinline bool is_physical(smg_uart_t* uart)
{
	return uart != nullptr && is_physical(uart->uart_nr);
}

/** @brief If given a virtual uart, obtain the related physical one
 */
smg_uart_t* get_physical(smg_uart_t* uart)
{
	return uart;
}

/** @brief UART interrupt service routine
 *  @note both UARTS share the same ISR, although UART1 only supports transmit
 */
void IRAM_ATTR uart_isr(smg_uart_instance_t* inst)
{
	if(inst == nullptr || inst->uart == nullptr) {
		return;
	}

	auto uart = inst->uart;
	auto dev = getDevice(uart->uart_nr);

	decltype(uart_dev_t::int_st) usis;
	usis.val = dev->int_st.val;

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
				size_t avail = uart_ll_get_rxfifo_len(dev);
				size_t space = uart->rx_buffer->getFreeSpace();
				read = (avail <= space) ? avail : space;
				space -= read;
				uint8_t buf[UART_RX_FIFO_SIZE];
				uart_ll_read_rxfifo(dev, buf, read);
				uint8_t* ptr = buf;
				while(read-- != 0) {
					uart->rx_buffer->writeChar(*ptr++);
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
				uart_ll_disable_intr_mask(dev, UART_INTR_RXFIFO_OVF);
			} else if(read == 0) {
				uart_ll_disable_intr_mask(dev, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT);
			}
		}

		// Unless we replenish TX FIFO, disable after handling interrupt
		if(usis.txfifo_empty) {
			// Dump as much data as we can from buffer into the TX FIFO
			if(uart->tx_buffer != nullptr) {
				size_t space = uart_txfifo_free(dev);
				size_t avail = uart->tx_buffer->available();
				size_t count = std::min(avail, space);
				uint8_t buf[count];
				for(unsigned i = 0; i < count; ++i) {
					buf[i] = uart->tx_buffer->readChar();
				}
				uart_ll_write_txfifo(dev, buf, count);
			}

			// If TX FIFO remains empty then we must disable TX FIFO EMPTY interrupt to stop it recurring.
			if(uart_txfifo_count(dev) == 0) {
				// The interrupt gets re-enabled by uart_write()
				uart_ll_disable_intr_mask(dev, UART_INTR_TXFIFO_EMPTY);
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
	dev->int_clr.val = usis.val;
}

} // namespace

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

void smg_uart_set_callback(smg_uart_t* uart, smg_uart_callback_t callback, void* param)
{
	if(uart != nullptr) {
		uart->callback = nullptr; // In case interrupt fires between setting param and callback
		uart->param = param;
		uart->callback = callback;
	}
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
		auto dev = getDevice(uart->uart_nr);
		auto len = std::min(uint32_t(size - read), uart_ll_get_rxfifo_len(dev));
		uart_ll_read_rxfifo(dev, &buf[read], len);
		read += len;

		// FIFO full may have been disabled if buffer overflowed, re-enabled it now
		uart_ll_clr_intsts_mask(dev, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT | UART_INTR_RXFIFO_OVF);
		uart_ll_ena_intr_mask(dev, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT | UART_INTR_RXFIFO_OVF);
	}

	return read;
}

size_t smg_uart_rx_available(smg_uart_t* uart)
{
	if(!smg_uart_rx_enabled(uart)) {
		return 0;
	}

	smg_uart_disable_interrupts();

	auto dev = getDevice(uart->uart_nr);
	size_t avail = is_physical(uart) ? uart_ll_get_rxfifo_len(dev) : 0;

	if(uart->rx_buffer != nullptr) {
		avail += uart->rx_buffer->available();
	}

	smg_uart_restore_interrupts();

	return avail;
}

void smg_uart_start_isr(smg_uart_t* uart)
{
	if(!is_physical(uart)) {
		return;
	}

	decltype(uart_dev_t::int_ena) int_ena{};

	auto dev = getDevice(uart->uart_nr);
	dev->conf1.val = 0;

	if(smg_uart_rx_enabled(uart)) {
		uart_ll_set_rxfifo_full_thr(dev, RX_FIFO_FULL_THRESHOLD);
		uart_ll_set_rx_tout(dev, 10);

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
		uart_ll_set_txfifo_empty_thr(dev, 10);
	}

	dev->int_clr.val = 0x0007ffff;
	dev->int_ena.val = int_ena.val;

	smg_uart_disable_interrupts();
	auto& inst = uartInstances[uart->uart_nr];
	auto& conn = uart_periph_signal[uart->uart_nr];
	esp_intr_alloc(conn.irq, ESP_INTR_FLAG_IRAM, intr_handler_t(uart_isr), &inst, &inst.handle);
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
				auto dev = getDevice(uart->uart_nr);
				auto len = std::min(size - written, uart_txfifo_free(dev));
				uart_ll_write_txfifo(dev, &buf[written], len);
				written += len;
				// Enable TX FIFO EMPTY interrupt
				uart_ll_clr_intsts_mask(dev, UART_INTR_TXFIFO_EMPTY);
				uart_ll_ena_intr_mask(dev, UART_INTR_TXFIFO_EMPTY);
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

	size_t space = is_physical(uart) ? uart_txfifo_free(getDevice(uart->uart_nr)) : 0;
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
		auto dev = getDevice(uart->uart_nr);
		while(uart_txfifo_count(dev) != 0)
			system_soft_wdt_feed();
	}
}

void smg_uart_set_break(smg_uart_t* uart, bool state)
{
	uart = get_physical(uart);
	if(uart != nullptr) {
		auto dev = getDevice(uart->uart_nr);
		dev->conf0.txd_brk = state;
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
			auto dev = getDevice(uart->uart_nr);
			decltype(uart_dev_t::int_raw) int_raw;
			int_raw.val = dev->int_raw.val;
			status.brk_det |= int_raw.brk_det;
			status.rxfifo_ovf |= int_raw.rxfifo_ovf;
			status.frm_err |= int_raw.frm_err;
			status.parity_err |= int_raw.parity_err;
			// Clear errors
			dev->int_clr.val = status.val;
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
		auto dev = getDevice(uart->uart_nr);

		if(flushTx) {
			// Prevent TX FIFO EMPTY interrupts - don't need them until uart_write is called again
			uart_ll_disable_intr_mask(dev, UART_INTR_TXFIFO_EMPTY);
			uart_ll_txfifo_rst(dev);
		}

		// If receive overflow occurred then these interrupts will be masked
		if(flushRx) {
			uart_ll_rxfifo_rst(dev);

			decltype(uart_dev_t::int_clr) int_clr;
			int_clr.val = 0x0007ffff;
			int_clr.txfifo_empty = false; // Leave this one
			dev->int_clr.val = int_clr.val;

			uart_ll_ena_intr_mask(dev, UART_INTR_RXFIFO_FULL | UART_INTR_RXFIFO_TOUT | UART_INTR_RXFIFO_OVF);
		}
	}

	smg_uart_restore_interrupts();
}

uint32_t smg_uart_set_baudrate_reg(int uart_nr, uint32_t baud_rate)
{
	if(!is_physical(uart_nr) || baud_rate == 0) {
		return 0;
	}

	auto dev = getDevice(uart_nr);

	uart_ll_set_sclk(dev, UART_SCLK_APB);

	// Return the actual baud rate in use
#if ESP_IDF_VERSION_MAJOR < 5
	uart_ll_set_baudrate(dev, baud_rate);
	return uart_ll_get_baudrate(dev);
#else
	uart_ll_set_baudrate(dev, baud_rate, APB_CLK_FREQ);
	return uart_ll_get_baudrate(dev, APB_CLK_FREQ);
#endif
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

	auto mem = heap_caps_malloc(sizeof(smg_uart_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
	auto uart = new(mem) smg_uart_t{};
	if(uart == nullptr) {
		return nullptr;
	}

	uart->uart_nr = cfg.uart_nr;
	uart->mode = cfg.mode;
	uart->options = cfg.options;
	uart->rx_headroom = DEFAULT_RX_HEADROOM;

	int tx_pin = cfg.tx_pin;
	int rx_pin = cfg.rx_pin;

	auto rxBufferSize = cfg.rx_size;
	auto txBufferSize = cfg.tx_size;

	if(smg_uart_rx_enabled(uart)) {
		if(!smg_uart_realloc_buffer(uart->rx_buffer, rxBufferSize)) {
			delete uart;
			return nullptr;
		}
		rx_pin = (cfg.rx_pin == UART_PIN_DEFAULT) ? defaultPins[cfg.uart_nr].rx : cfg.rx_pin;
	} else {
		rx_pin = UART_PIN_NO_CHANGE;
	}

	if(smg_uart_tx_enabled(uart)) {
		if(!smg_uart_realloc_buffer(uart->tx_buffer, txBufferSize)) {
			delete uart->rx_buffer;
			delete uart;
			return nullptr;
		}
		tx_pin = (tx_pin == UART_PIN_DEFAULT) ? defaultPins[cfg.uart_nr].tx : cfg.tx_pin;
	} else {
		tx_pin = UART_PIN_NO_CHANGE;
	}

	// OK, buffers allocated so setup hardware
	smg_uart_detach(cfg.uart_nr);
	smg_uart_set_pins(uart, tx_pin, rx_pin);

	auto& conn = uart_periph_signal[cfg.uart_nr];

	periph_module_enable(conn.module);

	auto dev = getDevice(cfg.uart_nr);

// Workaround for ESP32C3: enable core reset before enabling uart module clock to prevent uart output garbage value.
#if SOC_UART_REQUIRE_CORE_RESET
	uart_ll_set_reset_core(dev, true);
	periph_module_reset(conn.module);
	uart_ll_set_reset_core(dev, false);
#else
	periph_module_reset(conn.module);
#endif

	uart_ll_set_mode(dev, UART_MODE_UART);
	uart_ll_set_tx_idle_num(dev, 0);

	// Bottom 8 bits identical to esp8266
	dev->conf0.val = (dev->conf0.val & 0xFFFFFF00) | cfg.format;

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

	auto& conn = uart_periph_signal[uart->uart_nr];
	periph_module_disable(conn.module);

	uartInstances[uart->uart_nr].uart = nullptr;
	delete uart->rx_buffer;
	delete uart->tx_buffer;
	delete uart;
}

void smg_uart_set_format(smg_uart_t* uart, smg_uart_format_t format)
{
	uart = get_physical(uart);
	if(uart == nullptr) {
		return;
	}
	smg_uart_config_format_t fmt{.val = format};
	auto dev = getDevice(uart->uart_nr);
	uart_ll_set_data_bit_num(dev, uart_word_length_t(fmt.bits));
	uart_ll_set_parity(dev, uart_parity_t(fmt.parity));
	uart_ll_set_stop_bits(dev, uart_stop_bits_t(fmt.stop_bits));
}

bool smg_uart_intr_config(smg_uart_t* uart, const smg_uart_intr_config_t* config)
{
	uart = get_physical(uart);
	if(uart == nullptr || config == nullptr) {
		return false;
	}

	auto dev = getDevice(uart->uart_nr);
	if(smg_uart_rx_enabled(uart)) {
		uint8_t full_threshold;
		if(uart->rx_buffer == nullptr) {
			// Setting this to 0 results in lockup as the interrupt never clears
			full_threshold = TRange(1, UART_RXFIFO_FULL_THRHD).clip(config->rxfifo_full_thresh);
		} else {
			full_threshold = RX_FIFO_FULL_THRESHOLD;
		}
		uart_ll_set_rxfifo_full_thr(dev, full_threshold);
		uart_ll_set_rx_tout(dev, TRange(0, UART_RX_TOUT_THRHD).clip(config->rx_timeout_thresh));
	}

	if(smg_uart_tx_enabled(uart)) {
		uart_ll_set_txfifo_empty_thr(dev, TRange(0, UART_TXFIFO_EMPTY_THRHD).clip(config->txfifo_empty_intr_thresh));
	}

	dev->int_clr.val = config->intr_mask;
	dev->int_ena.val = (dev->int_ena.val & ~config->intr_mask) | config->intr_enable;

	return true;
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

	if(tx_pin != UART_PIN_NO_CHANGE && !GPIO_IS_VALID_OUTPUT_GPIO(tx_pin)) {
		return false;
	}

	if(rx_pin != UART_PIN_NO_CHANGE && !GPIO_IS_VALID_OUTPUT_GPIO(rx_pin)) {
		return false;
	}

	auto& conn = uart_periph_signal[uart->uart_nr];

	if(tx_pin != UART_PIN_NO_CHANGE) {
		gpio_ll_iomux_func_sel(GPIO_PIN_MUX_REG[tx_pin], PIN_FUNC_GPIO);
		gpio_set_level(gpio_num_t(tx_pin), true);
#ifdef SOC_UART_TX_PIN_IDX
		gpio_matrix_out(tx_pin, conn.pins[SOC_UART_TX_PIN_IDX].signal, false, false);
#else
		gpio_matrix_out(tx_pin, conn.tx_sig, false, false);
#endif
		uart->tx_pin = tx_pin;
	}

	if(rx_pin != UART_PIN_NO_CHANGE) {
		gpio_ll_iomux_func_sel(GPIO_PIN_MUX_REG[rx_pin], PIN_FUNC_GPIO);
		gpio_set_pull_mode(gpio_num_t(rx_pin), GPIO_PULLUP_ONLY);
		gpio_set_direction(gpio_num_t(rx_pin), GPIO_MODE_INPUT);
#ifdef SOC_UART_RX_PIN_IDX
		gpio_matrix_in(rx_pin, conn.pins[SOC_UART_RX_PIN_IDX].signal, false);
#else
		gpio_matrix_in(rx_pin, conn.rx_sig, false);
#endif
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

	auto dev = getDevice(uart_nr);
	dev->conf1.val = 0;
	dev->int_clr.val = 0x0007ffff;
	dev->int_ena.val = 0;
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
		auto dev = getDevice(uart_nr);
		// Wait for any outgoing data to finish sending (e.g. at boot time)
		while(uart_txfifo_count(dev) != 0) {
		}
		dev->conf1.val = 0;
		dev->int_clr.val = 0x0007ffff;
		dev->int_ena.val = 0;
	}
	isrMask = 0;
}
