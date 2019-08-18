/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * i2s.cpp - Hardware I2S driver stub
 *
 ****/

#include <driver/i2s.h>

bool i2s_driver_install(const i2s_config_t* config)
{
	return true;
}

void i2s_driver_uninstall()
{
}

bool i2s_start()
{
	return true;
}

bool i2s_stop()
{
	return false;
}

bool i2s_set_sample_rates(uint32_t rate)
{
	return true;
}

bool i2s_set_dividers(uint8_t bck_div, uint8_t mclk_div)
{
	return true;
}

float i2s_get_real_rate()
{
	return 0.0;
}

bool i2s_dma_read(i2s_buffer_info_t* info, size_t max_bytes)
{
	return false;
}

bool i2s_dma_write(i2s_buffer_info_t* info, size_t max_bytes)
{
	return false;
}

size_t i2s_write(const void* src, size_t size, TickType_t ticks_to_wait)
{
	return 0;
}

size_t i2s_read(void* dest, size_t size, TickType_t ticks_to_wait)
{
	return 0;
}

bool i2s_zero_dma_buffer()
{
	return true;
}

void i2s_set_pins(i2s_pin_set_t pins, bool enable)
{
}

bool i2s_enable_loopback(bool enable)
{
	return false;
}

bool i2s_stat_tx(i2s_buffer_stat_t* stat)
{
	return false;
}

bool i2s_stat_rx(i2s_buffer_stat_t* stat)
{
	return false;
}
