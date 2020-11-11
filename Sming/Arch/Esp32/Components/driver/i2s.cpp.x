/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * i2s.cpp - Hardware I2S driver for ESP8266
 *
 * @author Aug 2019 - mikee47 <mike@sillyhouse.net>
 *
 * Comparison with Arduino library and IDF driver
 *
 * 	- Code API rewritten in line with IDF driver
 * 	  - Add config struct for setup
 * 	  - Provide separate config for TX/RX to reflect hardware capability
 *	  - Add trigger threshold so callback frequency for 'TX/RX done' can be tuned
 *	  - Use single callback function with event code
 * 	  - Read/Write operates on byte buffers (easier to accommodate variable
 *      sample sizes and non-standard applications)
 * 	- Redundant copy operations eliminated, DMA buffers managed using indices
 * 	- Driver doesn't automatically configure I/O pins so as not to interfere
 * 	  with other peripherals. This is done separately using `i2s_set_pins()`
 * 	  or with regular pinMode() calls.
 * 	- Add `i2s_dma_write()` and `i2s_dma_read()` for zero-copy transfers
 * 	  without double-buffering.
 *	- Add `i2s_start()` and `is2_stop()` functions.
 *	- Add `i2s_zero_dma_buffer` to keep link active but muted. More efficient than
 *	  clearing buffer after every TX (though this can be enabled using a config flag).
 *
 *
 * ISSUES TO FIX
 *
 * 	What happens on buffer underrun? For example, we may wish to just stop.
 * 	Add this as event for callback.
 *
 * 	If stopped, make all buffers available for writing so when started data is
 * 	sent out precisely.
 *
 * 	Provide option to configure without DMA, just using FIFO. This might be useful
 * 	for sending short data bursts, e.g. writing to external shift register.
 *
 ****/

#include "include/driver/i2s.h"

#include <Platform/Timers.h>

#include <algorithm>
#include "esp_systemapi.h"
#include "espinc/pin_mux_register.h"
#include "espinc/i2c_bbpll.h"
#include "espinc/i2s_register.h"
#include "espinc/i2s_struct.h"
#include "espinc/slc_register.h"
#include "espinc/slc_struct.h"

#define I2S_CLK_ENABLE() i2c_writeReg_Mask_def(i2c_bbpll, i2c_bbpll_en_audio_clock_out, 1)
#define I2S_CLK_DISABLE() i2c_writeReg_Mask_def(i2c_bbpll, i2c_bbpll_en_audio_clock_out, 0)

// I2S has separate PLL independent from CPU
#define I2S_BASE_CLK (2 * APB_CLK_FREQ)

__forceinline static void dma_intr_enable()
{
	ETS_SLC_INTR_ENABLE();
}

__forceinline static void dma_intr_disable()
{
	ETS_SLC_INTR_DISABLE();
}

/*
 * We use a queue to keep track of the DMA buffers that are empty. The ISR
 * will push buffers to the back of the queue, the I2S transmitter will pull
 * them from the front and fill them. For ease, the queue will contain
 * *pointers* to the DMA buffers, not the data itself. The queue depth is
 * one smaller than the amount of buffers we have, because there's always a
 * buffer that is being used by the DMA subsystem *right now* and we don't
 * want to be able to write to that simultaneously.
 *
 * For RX, it's a little different.  The buffers in i2s_slc_queue are
 * placed onto the list when they're filled by DMA
*/

/**
 * SLC DMA buffer descriptor
 */
struct dma_descriptor_t {
	uint32_t blocksize : 12;
	uint32_t datalen : 12;
	uint32_t unused : 5;
	uint32_t sub_sof : 1;
	uint32_t eof : 1;
	volatile uint32_t owner : 1; // DMA can change this value
	uint32_t* buf_ptr;
	dma_descriptor_t* next_link_ptr;
};

class InterruptLock
{
public:
	__forceinline InterruptLock()
	{
		dma_intr_disable();
	}

	__forceinline ~InterruptLock()
	{
		dma_intr_enable();
	}
};

/*
 * State information consists of a set of DMA buffers.
 *
 * Transmission
 *		Initially all buffers are empty, so the output will be muted.
 *		When the first buffer has been sent, an interrupt fires. We obtain the 'finished'
 *		descriptor address, which is now released for use. New outgoing data then needs
 *		to be written. The initial write can fill up all buffers, except the active one (finished->next).
 *		If an underflow occurs, we lose sync. with the output stream. Until such time as more
 *		data is supplied, the buffer position must track behind the DMA position.
 *
 *	Reception
 *		Interrupt fires when data becomes available.
 *
 */
struct i2s_state_t {
	uint32_t* buffers;				   ///< Buffers allocated in single block
	dma_descriptor_t* slc_items;	   ///< DMA buffer descriptors
	volatile uint8_t buffers_used = 0; ///< Number of queued buffers
	volatile uint8_t buffer_index = 0; ///< Current buffer for user read/write
	uint16_t buffer_pos = 0;		   ///< Position in the current buffer for read/write
	uint16_t buffer_size = 0;		   ///< Size of each buffer (in bytes)
	uint8_t buffer_count = 0;		   ///< Number of buffers
	uint8_t callback_threshold = 0;	///< TX: callback when available buffers > threshold
									   ///< RX: Callback when buffers_used > threshold

	~i2s_state_t();

	bool initialise(const i2s_module_config_t& config);

	size_t user_size()
	{
		return (buffer_count - 1) * buffer_size;
	}

	size_t tx_used()
	{
		return (buffers_used * buffer_size) + buffer_pos;
	}

	size_t rx_available()
	{
		return (buffers_used * buffer_size) - buffer_pos;
	}

	bool IRAM_ATTR tx_done(dma_descriptor_t* desc);
	bool IRAM_ATTR rx_done(dma_descriptor_t* desc);

	bool IRAM_ATTR dma_write(i2s_buffer_info_t& info, size_t max_bytes);
	bool IRAM_ATTR dma_read(i2s_buffer_info_t& info, size_t max_bytes);

	size_t write(const void* src, size_t size, TickType_t ticks_to_wait);
	size_t read(void* dest, size_t size, TickType_t ticks_to_wait);
};

// RX = I2S receive (i.e. microphone), TX = I2S transmit (i.e. DAC)
struct i2s_object_t {
	i2s_state_t* rx_state = nullptr;
	i2s_state_t* tx_state = nullptr;
	i2s_callback_t callback = nullptr;
	void* param = nullptr;
	uint32_t sample_rate = 0;
	bool tx_desc_auto_clear = false; ///< I2S auto clear tx descriptor on underflow

	static volatile i2s_struct_t& i2s;
	static volatile slc_struct_t& dma;

	~i2s_object_t();

	bool initialise(const i2s_config_t& config);

	bool set_sample_rates(uint32_t rate);

	void i2s_reset_fifo()
	{
		i2s.conf.val &= ~I2S_I2S_RESET_MASK;
		i2s.conf.val |= I2S_I2S_RESET_MASK;
		i2s.conf.val &= ~I2S_I2S_RESET_MASK;
	}

	void reset_dma()
	{
		// Reset both links
		dma.conf0.val |= SLC_RXLINK_RST | SLC_TXLINK_RST;
		dma.conf0.val &= ~(SLC_RXLINK_RST | SLC_TXLINK_RST);
	}

	void enable_tx_intr(bool enable)
	{
		dma.int_ena.rx_eof = enable;
		dma.int_ena.rx_dscr_err = enable;
	}

	void enable_rx_intr(bool enable)
	{
		dma.int_ena.tx_suc_eof = enable;
		dma.int_ena.tx_dscr_err = enable;
	}

	void start();
	void stop();

	bool enable_loopback(bool enable);

	void IRAM_ATTR isr(uint32_t status);
};

volatile i2s_struct_t& i2s_object_t::i2s = I2S0;
volatile slc_struct_t& i2s_object_t::dma = SLC0;

static i2s_object_t* i2s_obj = nullptr;

__forceinline static bool rx_enabled()
{
	return i2s_obj != nullptr && i2s_obj->rx_state != nullptr;
}

__forceinline static bool tx_enabled()
{
	return i2s_obj != nullptr && i2s_obj->tx_state != nullptr;
}

bool i2s_stat_tx(i2s_buffer_stat_t* stat)
{
	if(!tx_enabled() || stat == nullptr) {
		return false;
	}

	InterruptLock lock;

	stat->size = i2s_obj->tx_state->user_size();
	stat->used = i2s_obj->tx_state->tx_used();
	return true;
}

bool i2s_stat_rx(i2s_buffer_stat_t* stat)
{
	if(!rx_enabled() || stat == nullptr) {
		return false;
	}

	InterruptLock lock;

	stat->size = i2s_obj->rx_state->user_size();
	stat->used = i2s_obj->rx_state->rx_available();
	return true;
}

bool i2s_state_t::tx_done(dma_descriptor_t* desc)
{
	if(i2s_obj->tx_desc_auto_clear) {
		// Zero the buffer so it's muted in case of underflow
		memset(desc->buf_ptr, 0x00, desc->datalen);
	}
	if(buffers_used == 0) {
		// All buffers are empty. This means we have an underflow
		unsigned buf_index = desc - slc_items;
		buffer_index = buf_index;
	} else {
		--buffers_used;
	}

	return (buffer_count - buffers_used) > callback_threshold;
}

bool i2s_state_t::rx_done(dma_descriptor_t* desc)
{
	// Set owner back to 1 (SW) or else RX stops.  TX has no such restriction.
	desc->owner = 1;
	if(buffers_used < (buffer_count - 1)) {
		++buffers_used;
	}

	return buffers_used > callback_threshold;
}

static void IRAM_ATTR i2s_slc_isr(void*)
{
	uint32_t status = SLC0.int_st.val;
	SLC0.int_clr.val = 0xFFFFFFFF;

	if(i2s_obj != nullptr) {
		i2s_obj->isr(status);
	}
}

void i2s_object_t::isr(uint32_t status)
{
	if(status & SLC_RX_EOF_INT_ST) {
		auto desc = reinterpret_cast<dma_descriptor_t*>(dma.rx_eof_des_addr);
		if(tx_state->tx_done(desc) && callback != nullptr) {
			callback(param, I2S_EVENT_TX_DONE);
		}
	}

	if(status & SLC_TX_EOF_INT_ST) {
		auto desc = reinterpret_cast<dma_descriptor_t*>(dma.tx_eof_des_addr);
		if(rx_state->rx_done(desc) && callback != nullptr) {
			callback(param, I2S_EVENT_RX_DONE);
		}
	}
}

bool i2s_state_t::initialise(const i2s_module_config_t& config)
{
	uint8_t bytes_per_sample = config.bits_per_sample / 8;
	uint8_t channel_num = (config.channel_format < I2S_CHANNEL_FMT_ONLY_RIGHT) ? 2 : 1;
	uint8_t sample_size = bytes_per_sample * channel_num;
	buffer_size = ALIGNUP4(config.dma_buf_len * sample_size);
	buffer_count = config.dma_buf_count;
	buffers = new uint32_t[buffer_count * buffer_size / sizeof(uint32_t)];
	slc_items = new dma_descriptor_t[buffer_count];
	if(buffers == nullptr || slc_items == nullptr) {
		return false;
	}

	memset(buffers, 0, buffer_count * buffer_size);
	memset(slc_items, 0, buffer_count);

	for(unsigned i = 0; i < buffer_count; ++i) {
		auto& item = slc_items[i];
		item.unused = 0;
		item.owner = 1;
		item.eof = 1;
		item.sub_sof = 0;
		item.datalen = buffer_size;
		item.blocksize = buffer_size;
		item.buf_ptr = buffers + (i * buffer_size / sizeof(uint32_t));
		item.next_link_ptr = &slc_items[(i + 1) % buffer_count];
	}

	return true;
}

i2s_state_t::~i2s_state_t()
{
	delete slc_items;
	delete buffers;
}

bool i2s_dma_write(i2s_buffer_info_t* info, size_t max_bytes)
{
	if(info == nullptr || !tx_enabled()) {
		return false;
	}

	return i2s_obj->tx_state->dma_write(*info, max_bytes);
}

bool i2s_state_t::dma_write(i2s_buffer_info_t& info, size_t max_bytes)
{
	// Room in the current buffer ?
	if(buffer_pos >= buffer_size) {
		// No. Any buffers available?
		if(buffers_used >= (buffer_count - 1)) {
			// No
			return false;
		}

		// Get a new buffer
		{
			InterruptLock lock;
			buffer_index = (buffer_index + 1) % buffer_count;
			++buffers_used;
		}
		buffer_pos = 0;
	}

	info.buf = buffer_index;
	info.pos = buffer_pos;

	info.buffer = reinterpret_cast<uint8_t*>(slc_items[buffer_index].buf_ptr) + buffer_pos;
	size_t size = buffer_size - buffer_pos;
	size = std::min(size, max_bytes);
	info.size = size;
	buffer_pos += size;

	return true;
}

size_t i2s_write(const void* src, size_t size, TickType_t ticks_to_wait)
{
	if(src == nullptr || !tx_enabled()) {
		return 0;
	}

	return i2s_obj->tx_state->write(src, size, ticks_to_wait);
}

size_t i2s_state_t::write(const void* src, size_t size, TickType_t ticks_to_wait)
{
	size_t count = 0;

	OneShotFastMs timer(ticks_to_wait);

	i2s_buffer_info_t info;
	auto buf = static_cast<const uint8_t*>(src);
	while(size > 0) {
		if(dma_write(info, size)) {
			memcpy(info.buffer, src, info.size);
			buf += info.size;
			size -= info.size;
			count += info.size;
			timer.start();
			continue;
		}

		if(timer.expired()) {
			break;
		}
	}

	return count;
}

bool i2s_zero_dma_buffer()
{
	if(!tx_enabled()) {
		return false;
	}

	i2s_buffer_info_t info;
	while(i2s_obj->tx_state->dma_write(info, UINT_MAX)) {
		memset(info.buffer, 0, info.size);
	}

	return true;
}

bool i2s_dma_read(i2s_buffer_info_t* info, size_t max_bytes)
{
	if(info == nullptr || !rx_enabled()) {
		return false;
	}

	return i2s_obj->rx_state->dma_read(*info, max_bytes);
}

bool i2s_state_t::dma_read(i2s_buffer_info_t& info, size_t max_bytes)
{
	// Read position at end of buffer?
	if(buffer_pos == buffer_size) {
		// Yes. Any more buffers to read?
		if(buffers_used == 0) {
			// No
			return false;
		}

		{
			InterruptLock lock;
			buffer_index = (buffer_index + 1) % buffer_count;
			--buffers_used;
		}
		buffer_pos = 0;
	}

	info.buf = buffer_index;
	info.pos = buffer_pos;

	info.buffer = reinterpret_cast<uint8_t*>(slc_items[buffer_index].buf_ptr) + buffer_pos;
	size_t size = buffer_size - buffer_pos;
	size = std::min(size, max_bytes);
	info.size = size;
	buffer_pos += size;

	return true;
}

size_t i2s_read(void* dest, size_t size, TickType_t ticks_to_wait)
{
	if(dest == nullptr || !rx_enabled()) {
		return 0;
	}

	return i2s_obj->rx_state->read(dest, size, ticks_to_wait);
}

size_t i2s_state_t::read(void* dest, size_t size, TickType_t ticks_to_wait)
{
	size_t count = 0;

	OneShotFastMs timer(ticks_to_wait);

	auto buf = static_cast<uint8_t*>(dest);
	i2s_buffer_info_t info;
	while(size > 0) {
		if(dma_read(info, size)) {
			memcpy(buf, info.buffer, info.size);
			buf += info.size;
			size -= info.size;
			count += info.size;
			timer.start();
			continue;
		}

		if(timer.expired()) {
			break;
		}
	}

	return count;
}

bool i2s_set_sample_rates(uint32_t rate)
{
	return (i2s_obj == nullptr) ? false : i2s_obj->set_sample_rates(rate);
}

bool i2s_object_t::set_sample_rates(uint32_t rate)
{
	if(rate == sample_rate) {
		return true;
	}

	uint8_t bck_div = 1;
	uint8_t mclk_div = 1;

	// Calculate the frequency division corresponding to the bit rate
	uint32_t scaled_base_freq = I2S_BASE_CLK / 32;
	float delta_best = scaled_base_freq;

	for(uint8_t i = 1; i < 64; i++) {
		for(uint8_t j = i; j < 64; j++) {
			float new_delta = abs(((float)scaled_base_freq / i / j) - rate);

			if(new_delta < delta_best) {
				delta_best = new_delta;
				bck_div = i;
				mclk_div = j;
			}
		}
	}

	if(!i2s_set_dividers(bck_div, mclk_div)) {
		return false;
	}

	sample_rate = rate;
	return true;
}

bool i2s_set_dividers(uint8_t bck_div, uint8_t mclk_div)
{
	if(bck_div == 0 || mclk_div == 0) {
		return false;
	}
	// Set I2S WS clock frequency. BCLK seems to be generated from 32x this
	auto& i2s = I2S0;
	i2s.conf.bck_div_num = bck_div;
	i2s.conf.clkm_div_num = mclk_div;
	return true;
}

float i2s_get_real_rate()
{
	auto& i2s = I2S0;
	return float(I2S_BASE_CLK) / (32 * i2s.conf.bck_div_num * i2s.conf.clkm_div_num);
}

bool i2s_enable_loopback(bool enable)
{
	return (i2s_obj == nullptr) ? false : i2s_obj->enable_loopback(enable);
}

bool i2s_object_t::enable_loopback(bool enable)
{
	dma.conf0.rx_loop_test = enable;
	return true;
}

static i2s_state_t* alloc_state(const i2s_module_config_t& config)
{
	auto state = new i2s_state_t;
	if(state == nullptr) {
		return nullptr;
	}

	if(!state->initialise(config)) {
		delete state;
		return nullptr;
	}

	state->callback_threshold = config.callback_threshold;
	return state;
}

bool i2s_driver_install(const i2s_config_t* config)
{
	if(i2s_obj != nullptr) {
		return false; // Already installed
	}

	if(config == nullptr) {
		return false;
	}

	i2s_obj = new i2s_object_t;
	if(i2s_obj != nullptr) {
		if(!i2s_obj->initialise(*config)) {
			delete i2s_obj;
			i2s_obj = nullptr;
		}
	}

	return i2s_obj != nullptr;
}

bool i2s_object_t::initialise(const i2s_config_t& config)
{
	callback = config.callback;
	param = config.param;

	if(config.tx.mode != I2S_MODE_DISABLED) {
		tx_state = alloc_state(config.tx);
		if(tx_state == nullptr) {
			return false;
		}
	}

	if(config.rx.mode != I2S_MODE_DISABLED) {
		rx_state = alloc_state(config.rx);
		if(rx_state == nullptr) {
			return false;
		}
	}

	stop();

	reset_dma();

	// Enable and configure DMA
	dma.conf0.txdata_burst_en = 0;
	dma.conf0.txdscr_burst_en = 1;
	dma.rx_dscr_conf.rx_fill_mode = 0;
	dma.rx_dscr_conf.rx_eof_mode = 0;
	dma.rx_dscr_conf.rx_fill_en = 0;
	dma.rx_dscr_conf.token_no_replace = 1;
	dma.rx_dscr_conf.infor_no_replace = 1;

	/*
	 * Feed DMA the first buffer desc addr
	 * To send data to the I2S subsystem, we use RXLINK (not the TXLINK as you might expect).
	 * The TXLINK part still needs a valid DMA descriptor, even if it's unused: the DMA engine will throw
	 * an error at us otherwise. Just feed it any random descriptor.
	*/
	if(rx_state == nullptr) {
		dma.tx_link.addr = uint32_t(&tx_state->slc_items[1]); // Set fake (unused) RX descriptor address
	} else {
		dma.tx_link.addr = uint32_t(&rx_state->slc_items[0]); // Set real RX address
	}
	if(tx_state == nullptr) {
		dma.rx_link.addr = uint32_t(&rx_state->slc_items[1]); // Set fake (unused) TX descriptor address
	} else {
		dma.rx_link.addr = uint32_t(&tx_state->slc_items[0]); // Set real TX address
	}

	// Attach handler
	ETS_SLC_INTR_ATTACH(i2s_slc_isr, nullptr);

	I2S_CLK_ENABLE();

	i2s_reset_fifo();

	// Disable all i2s interrupts
	i2s.int_ena.val = 0;

	reset_dma();

	/*
	 * FIFO modes, controls transport data format (bpc = bits per channel)
	 * 	0: 16bpc full data (dual channel)
	 * 	1: 16bpc half-data (single channel)
	 * 	2: 24bpc full data discontinue (dual channel)
	 * 	3: 24bpc half data discontinue (single channel)
	 * 	4: 24bpc full data continue (dual channel)
	 *  5: 24bpc half data continue (single channel)
	 *
	 * Note: Modes 4 & 5 TX only
	 * @todo: How to accommodate mode 4 & 5. Should driver interface be closer to H/W?
	 */
	auto get_fifo_mode = [](const i2s_module_config_t& cfg) -> uint8_t {
		uint8_t mode = (cfg.channel_format < I2S_CHANNEL_FMT_ONLY_RIGHT) ? 0x00 : 0x01;
		if(cfg.bits_per_sample > 16) {
			mode |= 0x02;
		}
		return mode;
	};

	/*
	 * Channel modes
	 *
	 * 	0: Dual-channel mode
	 * 	1: Single-channel mode, both left
	 * 	2: Single-channel mode, both right
	 * 	3: Single-channel mode, only left
	 * 	4: Single-channel mode, only right
	 *
	 * Modes 3 & 4 are TX-only
	 */
	auto get_channel_mode = [](const i2s_module_config_t& cfg) -> uint8_t {
		if(cfg.channel_format < I2S_CHANNEL_FMT_ONLY_RIGHT) {
			return cfg.channel_format;
		} else {
			return cfg.channel_format >> 1;
		}
	};

	// Configure FIFO for DMA
	i2s.fifo_conf.dscr_en = 0;
	i2s.conf_chan.tx_chan_mod = config.tx.channel_format;
	i2s.fifo_conf.tx_fifo_mod = get_fifo_mode(config.tx);
	i2s.conf_chan.rx_chan_mod = (config.rx.channel_format < I2S_CHANNEL_FMT_ONLY_RIGHT)
									? config.rx.channel_format
									: config.rx.channel_format >> 1; // RX doesn't directly support ONLY L/R
	i2s.fifo_conf.rx_fifo_mod = get_fifo_mode(config.rx);
	i2s.fifo_conf.dscr_en = 1; // Connect DMA to FIFO

	i2s.conf.tx_start = 0;
	i2s.conf.rx_start = 0;

	// Fix config. to MSB first, right channel first
	i2s.conf.msb_right = 1;
	i2s.conf.right_first = 1;

	// 1-bit delay from WS to MSB (I2S format)
	//	i2s.conf.rx_msb_shift = 1;
	//	i2s.conf.tx_msb_shift = 1;

	if(tx_state != nullptr) {
		i2s.conf.tx_msb_shift = (config.tx.communication_format & I2S_COMM_FORMAT_I2S_LSB) ? 1 : 0;
		i2s.conf.tx_slave_mod = (config.tx.mode == I2S_MODE_SLAVE) ? 1 : 0;
	}

	if(rx_state != nullptr) {
		i2s.conf.rx_msb_shift = (config.rx.communication_format & I2S_COMM_FORMAT_I2S_LSB) ? 1 : 0;
		i2s.conf.rx_slave_mod = (config.rx.mode == I2S_MODE_SLAVE) ? 1 : 0;
		// Need to prime the # of samples to receive in the engine
		i2s.rx_eof_num = rx_state->buffer_size;
	}

	// Bit length for 'third stage'
	// @todo experiment to establish exact behaviour and document
	// suspect source data can be in 16 or 32-bit blocks, this determining significant bits
	i2s.conf.bits_mod = config.bits_mod;

	set_sample_rates(config.sample_rate);

	if(config.auto_start) {
		start();
	}

	return true;
}

bool i2s_start()
{
	if(i2s_obj == nullptr) {
		return false;
	}

	i2s_obj->start();
	return true;
}

void i2s_object_t::start()
{
	dma_intr_disable();

	i2s_reset_fifo();
	reset_dma();

	i2s.conf.tx_reset = 1;
	i2s.conf.tx_reset = 0;
	i2s.conf.rx_reset = 1;
	i2s.conf.rx_reset = 0;

	if(tx_state != nullptr) {
		enable_tx_intr(true);
		dma.rx_link.start = 1;
	}

	if(rx_state != nullptr) {
		enable_rx_intr(true);
		dma.tx_link.start = 1;
	}

	// Both TX and RX are started to ensure clock generation
	i2s.conf.val |= I2S_I2S_TX_START | I2S_I2S_RX_START | I2S_I2S_RESET_MASK;
	// Simultaneously bring out of reset to ensure the same phase
	i2s.conf.val &= ~I2S_I2S_RESET_MASK;

	dma_intr_enable();
}

bool i2s_stop()
{
	if(i2s_obj == nullptr) {
		return false;
	}

	i2s_obj->stop();
	return true;
}

void i2s_object_t::stop()
{
	dma_intr_disable();

	if(tx_state != nullptr) {
		dma.rx_link.stop = 1;
		enable_tx_intr(false);
	}

	if(rx_state != nullptr) {
		dma.tx_link.stop = 1;
		enable_rx_intr(false);
	}

	i2s.conf.val &= ~(I2S_I2S_TX_START | I2S_I2S_RX_START);
	dma.int_clr.val = dma.int_st.val; // clear pending interrupt
}

void i2s_driver_uninstall()
{
	if(i2s_obj != nullptr) {
		delete i2s_obj;
	}
}

i2s_object_t::~i2s_object_t()
{
	stop();

	i2s_reset_fifo();

	dma.int_ena.val = 0;
	dma.tx_link.addr = 0;
	dma.tx_link.addr = 0;

	delete tx_state;
	delete rx_state;
}

void i2s_set_pins(i2s_pin_set_t pins, bool enable)
{
	if(pins & I2S_PIN_BCK_OUT) {
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, enable ? FUNC_I2SO_BCK : FUNC_GPIO15);
	}

	if(pins & I2S_PIN_WS_OUT) {
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, enable ? FUNC_I2SO_WS : FUNC_GPIO2);
	}

	if(pins & I2S_PIN_DATA_OUT) {
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, enable ? FUNC_I2SO_DATA : FUNC_U0RXD);
	}

	if(pins & I2S_PIN_BC_IN) {
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, enable ? FUNC_I2SI_BCK : FUNC_GPIO13);
	}

	if(pins & I2S_PIN_WS_IN) {
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, enable ? FUNC_I2SI_WS : FUNC_GPIO14);
	}

	if(pins & I2S_PIN_DATA_IN) {
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, enable ? FUNC_I2SI_DATA : FUNC_GPIO12);
	}
}
