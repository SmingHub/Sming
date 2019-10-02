/*
  core_esp8266_is2.cpp - Software I2S library for esp8266

  Arduino compatibility layer, uses Sming driver
*/

#include "i2s.h"
#include <driver/i2s.h>
#include <esp_attr.h>
#include <Platform/Timers.h>
#include <string.h>

#define SLC_BUF_CNT (3)   // Number of buffers in the I2S circular buffer
#define SLC_BUF_LEN (128) // Length of one buffer, in 32-bit words.

// Last I2S sample rate requested
static struct {
	uint32_t sample_rate;
	void (*tx_callback)();
	void (*rx_callback)();
} i2s_data;

bool i2s_is_full()
{
	i2s_buffer_stat_t stat;
	return i2s_stat_tx(&stat) ? (stat.used == stat.size) : true;
}

bool i2s_rx_is_full()
{
	i2s_buffer_stat_t stat;
	return i2s_stat_rx(&stat) ? (stat.used == stat.size) : false;
}

bool i2s_is_empty()
{
	i2s_buffer_stat_t stat;
	return i2s_stat_tx(&stat) ? (stat.used == 0) : true;
}

bool i2s_rx_is_empty()
{
	i2s_buffer_stat_t stat;
	return i2s_stat_rx(&stat) ? (stat.used == 0) : true;
}

uint16_t i2s_available()
{
	i2s_buffer_stat_t stat;
	return i2s_stat_tx(&stat) ? (stat.size - stat.used) : 0;
}

uint16_t i2s_rx_available()
{
	i2s_buffer_stat_t stat;
	return i2s_stat_rx(&stat) ? stat.used : 0;
}

void i2s_set_callback(void (*callback)(void))
{
	i2s_data.tx_callback = callback;
}

void i2s_rx_set_callback(void (*callback)(void))
{
	i2s_data.rx_callback = callback;
}

bool i2s_write_lr(int16_t left, int16_t right)
{
	i2s_sample_t sample;
	sample.left = left;
	sample.right = right;
	return i2s_write_sample(sample.u32);
}

// writes a buffer of frames into the DMA memory, returns the amount of frames written
// A frame is just a int16_t for mono, for stereo a frame is two int16_t, one for each channel
static uint16_t write_buffer(int16_t* frames, uint16_t frame_count, bool mono, bool nb)
{
	OneShotFastMs timer;
	if(!nb) {
		timer.reset<I2S_BLOCKING_TIMEOUT>();
	}

	i2s_buffer_info_t info;
	auto get_buffer = [&]() {
		do {
			if(i2s_dma_write(&info, frame_count * sizeof(i2s_sample_t))) {
				return true;
			}
		} while(!timer.expired());
		return false;
	};

	uint16_t frames_written = 0;
	while(frame_count > 0 && get_buffer()) {
		uint16_t fc = info.size / sizeof(i2s_sample_t);

		i2s_sample_t sample;
		if(mono) {
			for(uint16_t i = 0; i < fc; i++) {
				sample.left = *frames++;
				sample.right = sample.left;
				info.samples[i] = sample;
			}
		} else {
			for(uint16_t i = 0; i < fc; i++) {
				sample.left = *frames++;
				sample.right = *frames++;
				info.samples[i] = sample;
			}
		}

		frame_count -= fc;
		frames_written += fc;
	}
	return frames_written;
}

uint16_t i2s_write_buffer_mono_nb(int16_t* frames, uint16_t frame_count)
{
	return write_buffer(frames, frame_count, true, true);
}

uint16_t i2s_write_buffer_mono(int16_t* frames, uint16_t frame_count)
{
	return write_buffer(frames, frame_count, true, false);
}

uint16_t i2s_write_buffer_nb(int16_t* frames, uint16_t frame_count)
{
	return write_buffer(frames, frame_count, false, true);
}

uint16_t i2s_write_buffer(int16_t* frames, uint16_t frame_count)
{
	return write_buffer(frames, frame_count, false, false);
}

bool i2s_read_sample(int16_t* left, int16_t* right, bool blocking)
{
	i2s_sample_t sample;
	if(i2s_read(&sample, sizeof(sample), blocking ? I2S_BLOCKING_TIMEOUT : 0) != sizeof(sample)) {
		return false;
	}

	if(left != nullptr) {
		*left = sample.left;
	}
	if(right != nullptr) {
		*right = sample.right;
	}

	return true;
}

static void IRAM_ATTR i2s_callback(void* param, i2s_event_type_t event)
{
	switch(event) {
	case I2S_EVENT_TX_DONE:
		if(i2s_data.tx_callback != nullptr) {
			i2s_data.tx_callback();
		}
		break;
	case I2S_EVENT_RX_DONE:
		if(i2s_data.rx_callback != nullptr) {
			i2s_data.rx_callback();
		}
		break;
	default:; // ignore
	}
}

bool i2s_rxtx_begin(bool enableRx, bool enableTx)
{
	i2s_config_t config;
	memset(&config, 0, sizeof(config));
	i2s_module_config_t modcfg = {
		.mode = I2S_MODE_MASTER,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format = I2S_COMM_FORMAT_I2S_MSB,
		.dma_buf_len = SLC_BUF_LEN * 2, // in samples
		.dma_buf_count = SLC_BUF_CNT,
		.callback_threshold = 0,
	};
	if(enableRx) {
		config.rx = modcfg;
	}
	if(enableTx) {
		config.tx = modcfg;
	}
	config.sample_rate = 44100;
	config.tx_desc_auto_clear = true;
	config.auto_start = true;
	config.callback = i2s_callback;
	return i2s_driver_install(&config);
}
