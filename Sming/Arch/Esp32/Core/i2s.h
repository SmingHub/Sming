/*
  i2s.h - Software I2S library for esp8266

  (Sming Arduino compatility layer)

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef I2S_h
#define I2S_h

#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/i2s.h>

/*
How does this work? Basically, to get sound, you need to:
- Connect an I2S codec to the I2S pins on the ESP.
- Start up a thread that's going to do the sound output
- Call i2s_begin()
- Call i2s_set_rate() with the sample rate you want.
- Generate sound and call i2s_write_sample() with 32-bit samples.
The 32bit samples basically are 2 16-bit signed values (the analog values for
the left and right channel) concatenated as (Rout<<16)+Lout

i2s_write_sample will block when you're sending data too quickly, so you can just
generate and push data as fast as you can and i2s_write_sample will regulate the
speed.
*/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Blocking calls are time-limited (in milliseconds)
#define I2S_BLOCKING_TIMEOUT 100

/**
 * @brief
 */
bool i2s_rxtx_begin(bool enableRx, bool enableTx);

/**
 * @brief Enable TX only, for compatibility
 */
inline bool i2s_begin()
{
	return i2s_rxtx_begin(false, true);
}

/**
 * @brief
 */
inline void i2s_end()
{
	// TODO:
	//	i2s_driver_uninstall(PORT);
}

/**
 * @param rate Sample rate in Hz (ex 44100, 48000) for TX/RX
 */
inline bool i2s_set_rate(uint32_t rate)
{
	// TODO:
	//	return i2s_set_sample_rates(PORT, rate);
	return false;
}

/*
 * These functions are implemented in the driver itself
 */
// bool i2s_set_dividers(uint8_t bck_div, uint8_t mclk_div);
// float i2s_get_real_rate();

bool i2s_is_full();

bool i2s_rx_is_full();

bool i2s_is_empty();

bool i2s_rx_is_empty();

uint16_t i2s_available();

uint16_t i2s_rx_available();

/**
 * @brief 32bit sample with channels being upper and lower 16 bits (blocking when DMA is full)
 */
inline bool i2s_write_sample(uint32_t sample)
{
	//	return i2s_write(PORT, &sample, sizeof(sample), I2S_BLOCKING_TIMEOUT) == sizeof(sample);
	return false;
}

/**
 * @brief Same as above but does not block when DMA is full and returns false instead
 */
inline bool i2s_write_sample_nb(uint32_t sample)
{
	//	return i2s_write(PORT, &sample, sizeof(sample), 0) == sizeof(sample);
	return false;
}

/**
 * @brief
 * @note Combines both channels and calls i2s_write_sample with the result
 */
bool i2s_write_lr(int16_t left, int16_t right);

/**
 * @brief
 * @note RX data returned in both 16-bit outputs.
 */
bool i2s_read_sample(int16_t* left, int16_t* right, bool blocking);

/**
 * @brief
 * @retval bool true if DMA is full and can not take more bytes (overflow)
 */
bool i2s_is_full();

/**
 * @brief
 * @retval bool true if DMA is empty (underflow)
 */
bool i2s_is_empty();

/**
 * @brief
 */
bool i2s_rx_is_full();

/**
 * @brief
 */
bool i2s_rx_is_empty();

/**
 * @brief
 * @retval uint16_t Number of samples than can be written before blocking
 */
uint16_t i2s_available();

/**
 * @brief
 * @retval uint16_t Number of samples that can be read before blocking
 */
uint16_t i2s_rx_available();

/**
 * @brief
 */
void i2s_set_callback(void (*callback)(void));

/**
 * @brief
 */
void i2s_rx_set_callback(void (*callback)(void));

/**
 * @brief Writes a buffer of frames into the DMA memory, returns the amount of frames written.
 * @note A frame is just a int16_t for mono, for stereo a frame is two int16_t, one for each channel.
 */
uint16_t i2s_write_buffer_mono(int16_t* frames, uint16_t frame_count);

/**
 * @brief
 */
uint16_t i2s_write_buffer_mono_nb(int16_t* frames, uint16_t frame_count);

/**
 * @brief
 */
uint16_t i2s_write_buffer(int16_t* frames, uint16_t frame_count);

/**
 * @brief
 */
uint16_t i2s_write_buffer_nb(int16_t* frames, uint16_t frame_count);

#ifdef __cplusplus
}
#endif

#endif
