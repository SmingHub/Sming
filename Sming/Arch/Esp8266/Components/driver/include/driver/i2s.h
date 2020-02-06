/**
 * i2s.h - Hardware I2S driver for ESP8266
 *
 * @author Aug 2019 - mikee47 <mike@sillyhouse.net>
 *
 * Fully reworked from the Arduino Esp8266 library
 * 	 Copyright (c) 2015 Hristo Gochkov. All rights reserved.
 *
 * And the ESP8266 RTOS driver,
 *	Copyright 2018-2025 Espressif Systems (Shanghai) PTE LTD
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with Sming.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <esp_attr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup i2s_driver I2S Driver
 * @ingroup drivers
 * @{
 */

/**
 * @brief I2S sample
 *
 * An I2S frame can contain various types of data:
 *
 * 8-bit, 16-bit or 24-bit mono samples
 * 8-bit or 16-bit stereo samples
 *
 */
typedef union {
	uint32_t u32;
	struct {
		int16_t left;
		int16_t right;
	};
} i2s_sample_t;

/**
 * @brief I2S bit width per sample
 */
typedef enum {
	I2S_BITS_PER_SAMPLE_8BIT = 8,   ///< I2S bits per sample: 8-bits
	I2S_BITS_PER_SAMPLE_16BIT = 16, ///< I2S bits per sample: 16-bits
	I2S_BITS_PER_SAMPLE_24BIT = 24, ///< I2S bits per sample: 24-bits
} i2s_bits_per_sample_t;

/**
 * @brief I2S channel
 */
typedef enum {
	I2S_CHANNEL_MONO = 1,  ///< I2S 1 channel (mono)
	I2S_CHANNEL_STEREO = 2 ///< I2S 2 channel (stereo)
} i2s_channel_t;

/**
 * @brief I2S communication standard format
 */
typedef enum {
	I2S_COMM_FORMAT_I2S = 0x01,		///< I2S communication format I2S
	I2S_COMM_FORMAT_I2S_MSB = 0x01, ///< I2S format MSB
	I2S_COMM_FORMAT_I2S_LSB = 0x03, ///< I2S format LSB
} i2s_comm_format_t;

/**
 * @brief I2S channel format type
 */
typedef enum {
	I2S_CHANNEL_FMT_RIGHT_LEFT = 0x00,
	I2S_CHANNEL_FMT_ALL_RIGHT,
	I2S_CHANNEL_FMT_ALL_LEFT,
	I2S_CHANNEL_FMT_ONLY_RIGHT,
	I2S_CHANNEL_FMT_ONLY_LEFT,
} i2s_channel_fmt_t;

/**
 * @brief I2S Mode, default is I2S_MODE_MASTER
 */
typedef enum {
	I2S_MODE_DISABLED,
	I2S_MODE_MASTER,
	I2S_MODE_SLAVE,
} i2s_mode_t;

/**
 * @brief I2S event types
 */
typedef enum {
	I2S_EVENT_DMA_ERROR,
	I2S_EVENT_TX_DONE, /*!< I2S DMA finish sent 1 buffer*/
	I2S_EVENT_RX_DONE, /*!< I2S DMA finish received 1 buffer*/
	I2S_EVENT_MAX,	 /*!< I2S event max index*/
} i2s_event_type_t;

/**
 * @brief Callback function type
 * @note Function is called in interrupt context, so place in IRAM and keep it brief.
 */
typedef void (*i2s_callback_t)(void* param, i2s_event_type_t event);

/**
 * @brief I2S module configuration (TX or RX)
 */
typedef struct {
	i2s_mode_t mode;						///< I2S work mode (combination of i2s_mode_t)
	i2s_bits_per_sample_t bits_per_sample;  ///< I2S bits per sample
	i2s_channel_fmt_t channel_format;		///< I2S channel format
	i2s_comm_format_t communication_format; ///< I2S communication format
	uint16_t dma_buf_len;					///< I2S DMA Buffer Length (in samples)
	uint8_t dma_buf_count;					///< I2S DMA Buffer Count
	uint8_t callback_threshold;				///< TX: callback when available buffers > threshold
											///< RX: Callback when slc_queue_len > threshold
} i2s_module_config_t;

/**
 * @brief I2S configuration parameters
 */
typedef struct {
	i2s_module_config_t tx;  ///< TX module configuration
	i2s_module_config_t rx;  ///< RX module configuration
	unsigned sample_rate;	///< I2S sample rate
	bool tx_desc_auto_clear; ///< I2S auto clear tx descriptor if there is underflow condition (Mutes output)
	bool auto_start;		 ///< Start immediately on successful initialisation
	i2s_callback_t callback; ///< Callback handler
	void* param;			 ///< Callback parameter
	uint8_t bits_mod;		 ///< Evaluate what this does (4 bits)
} i2s_config_t;

/**
 * @brief Install and start I2S driver
 * @note  This function must be called before any I2S driver read/write operations.
 * @param config I2S configuration
 * @retval true on success, false if already installed or invalid config
 */
bool i2s_driver_install(const i2s_config_t* config);

/**
 * @brief Uninstall I2S driver.
 */
void i2s_driver_uninstall();

/**
 * @brief Start I2S driver
 * @note It is not necessary to call this function after i2s_driver_install() as it is started automatically,
 * unless `config.auto_start` was set to false.
 * @retval bool true on success, false if driver not initialised
 */
bool i2s_start();

/**
 * @brief Stop I2S driver
 * @note Disables I2S TX/RX, until i2s_start() is called
 * @retval bool true on success, false if driver not initialised
 */
bool i2s_stop();

/**
 * @param rate Sample rate in Hz (ex 44100, 48000) for TX/RX
 */
bool i2s_set_sample_rates(uint32_t rate);

/**
 * Direct control over output rate
 */
bool i2s_set_dividers(uint8_t bck_div, uint8_t mclk_div);

/**
 * @retval float The actual Sample Rate on output
 */
float i2s_get_real_rate();

/**
 * @brief Defines a buffer with available content
 */
typedef struct {
	union {
		void* buffer;
		i2s_sample_t* samples;
	};
	size_t size; ///< Available space (TX) or data (RX) in bytes
	// debugging
	uint16_t buf;
	uint16_t pos;
} i2s_buffer_info_t;

/**
 * @brief Defines the wait interval (presently milliseconds)
 */
typedef unsigned TickType_t;

/**
 * @brief Fetch a DMA buffer containing received data (zero-copy)
 * @param info Pointer to structure to receive buffer information
 * @param max_bytes Number of bytes to read
 * @retval bool true on success, false if no data available or info is null
 * @note On success, `info->buffer` specifies where to read the data from,
 * and `info->size` how many bytes are actually available (always > 0).
 * @note Returns at most one DMA buffer
 */
bool IRAM_ATTR i2s_dma_read(i2s_buffer_info_t* info, size_t max_bytes);

/**
 * @brief Fetch a DMA buffer for direct writing (zero-copy)
 * @param info Pointer to structure to receive buffer information
 * @param max_bytes Number of bytes required in buffer
 * @retval bool true on success, false if buffer unavailable or info is null
 * @note On success, `info->buffer` specifies where to write the data, and `info->size`
 * how many bytes should be written - may be less than max_bytes, but always > 0.
 * @note Returns at most one DMA buffer
 */
bool IRAM_ATTR i2s_dma_write(i2s_buffer_info_t* info, size_t max_bytes);

/**
 * @brief writes a buffer of frames into the DMA memory, returns the amount of frames written.
 * @param src Data to write
 * @param size Size in bytes
 * @param ticks_to_wait Wait timeout in ticks
 * @retval size_t Data actually written, may be less than size
 * @note Data is copied into DMA buffers
 */
size_t i2s_write(const void* src, size_t size, TickType_t ticks_to_wait);

/**
 * @brief Reads a block of received data.
 * @param dest Buffer to store data
 * @param size Max. bytes to read
 * @param ticks_to_wait Wait timeout in ticks
 * @retval size_t Number of bytes read
 */
size_t i2s_read(void* dest, size_t size, TickType_t ticks_to_wait);

/**
 * @brief Zero the contents of the TX DMA buffer
 * @note Pushes zero-byte samples into the TX DMA buffer, until it is full
 */
bool i2s_zero_dma_buffer();

/**
 * @brief I2S pin enable for i2s_set_pin
 */
typedef enum {
	I2S_PIN_BCK_OUT = 0x01,  ///< GPIO 15 / TXD2 / D8
	I2S_PIN_WS_OUT = 0x02,   ///< GPIO 2 / TXD1 / D4
	I2S_PIN_DATA_OUT = 0x04, ///< GPIO 3 / RXD0 / D9
	I2S_PIN_BC_IN = 0x10,	///< GPIO 13 / RXD2 / D7
	I2S_PIN_WS_IN = 0x20,	///< GPIO 14 / D5
	I2S_PIN_DATA_IN = 0x40,  ///< GPIO 12 / D6
} i2s_pin_t;

typedef uint8_t i2s_pin_set_t;

/**
 * @brief Configure I2S pins
 * @param pins Mask of i2s_pin_t values
 * @param enable true to enable for I2S use, false to revert to GPIO
 * @note Call this after initialising driver to specify which pins are required
 *
 * You can alternatively use arduino functions.
 *
 * Example: i2s_set_pins(_BV(I2S_BCK_OUT), true)
 */
void i2s_set_pins(i2s_pin_set_t pins, bool enable);

bool i2s_enable_loopback(bool enable);

/**
 * @brief Contains I2S buffer status information
 * @note Size excludes buffer in use by DMA
 */
typedef struct {
	uint16_t size;
	uint16_t used;
} i2s_buffer_stat_t;

/**
 * @brief Obtain state information for TX buffers
 * @param stat
 * @retval bool true on success
 */
bool i2s_stat_tx(i2s_buffer_stat_t* stat);

/**
 * @brief Obtain state information for RX buffers
 * @param stat
 * @retval bool true on success
 */
bool i2s_stat_rx(i2s_buffer_stat_t* stat);

/**
 * @retval unsigned Number of frames available to read
 */
//unsigned i2s_rx_available();


/** @} */

#ifdef __cplusplus
}
#endif
