/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.cpp
 *
 ****/

#include "driver/driver_api.h"
#include <Digital.h>
#include <esp_attr.h>
#include <esp32/rom/ets_sys.h>
#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <esp32/rom/gpio.h>
#include <soc/gpio_reg.h>
#include <soc/io_mux_reg.h>
#include <soc/gpio_struct.h>
#include <soc/rtc_io_reg.h>

const int8_t esp32_adc2gpio[20] = {36, 37, 38, 39, 32, 33, 34, 35, -1, -1, 4, 0, 2, 15, 13, 12, 14, 27, 25, 26};

const DRAM_ATTR esp32_gpioMux_t esp32_gpioMux[40] = {
	{0x44, 11, 11, 1},  {0x88, -1, -1, -1}, {0x40, 12, 12, 2},  {0x84, -1, -1, -1}, {0x48, 10, 10, 0},
	{0x6c, -1, -1, -1}, {0x60, -1, -1, -1}, {0x64, -1, -1, -1}, {0x68, -1, -1, -1}, {0x54, -1, -1, -1},
	{0x58, -1, -1, -1}, {0x5c, -1, -1, -1}, {0x34, 15, 15, 5},  {0x38, 14, 14, 4},  {0x30, 16, 16, 6},
	{0x3c, 13, 13, 3},  {0x4c, -1, -1, -1}, {0x50, -1, -1, -1}, {0x70, -1, -1, -1}, {0x74, -1, -1, -1},
	{0x78, -1, -1, -1}, {0x7c, -1, -1, -1}, {0x80, -1, -1, -1}, {0x8c, -1, -1, -1}, {0, -1, -1, -1},
	{0x24, 6, 18, -1}, //DAC1
	{0x28, 7, 19, -1}, //DAC2
	{0x2c, 17, 17, 7},  {0, -1, -1, -1},	{0, -1, -1, -1},	{0, -1, -1, -1},	{0, -1, -1, -1},
	{0x1c, 9, 4, 8},	{0x20, 8, 5, 9},	{0x14, 4, 6, -1},   {0x18, 5, 7, -1},   {0x04, 0, 0, -1},
	{0x08, 1, 1, -1},   {0x0c, 2, 2, -1},   {0x10, 3, 3, -1}};

void pinMode(uint16_t pin, uint8_t mode)
{
	// remap the mode to ESP32 mode.
	switch(mode) {
	case OUTPUT:
		mode = ESP32_OUTPUT;
		break;
	case INPUT:
		mode = ESP32_INPUT;
		break;
		// TODO: add the other modes if needed..
	}
	gpio_pad_select_gpio((uint8_t)pin + 1);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction((gpio_num_t)pin, (gpio_mode_t)mode);

	if(mode & INPUT_PULLUP) {
		gpio_pullup_en((gpio_num_t)pin);
	} else {
		gpio_pullup_dis((gpio_num_t)pin);
	}

	// TODO: Pull-down
	//	if (mode & INPUT_PULLDOWN) {
	//		gpio_pulldown_en((gpio_num_t)pin);
	//	}
	//	else {
	//		gpio_pulldown_dis((gpio_num_t)pin);
	//	}
}

//Detect if pin is input
bool isInputPin(uint16_t pin)
{
	gpio_num_t pinNumber = (gpio_num_t)(pin & 0x1F);

	if(GPIO_REG_READ(GPIO_ENABLE_REG) & BIT(pinNumber)) {
		//pin is output - read the GPIO_OUT_REG register
		return false;
	}

	return true;
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	gpio_set_level((gpio_num_t)pin, val);
}

uint8_t IRAM_ATTR digitalRead(uint16_t pin)
{
	return gpio_get_level((gpio_num_t)pin);
}

void pullup(uint16_t pin)
{
	gpio_pullup_en((gpio_num_t)pin);
}

void noPullup(uint16_t pin)
{
	gpio_pullup_dis((gpio_num_t)pin);
}

/* Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure.
 * Max timeout is 27 seconds at 160MHz clock and 54 seconds at 80MHz clock */
#define WAIT_FOR_PIN_STATE(state)                                                                                      \
	while(digitalRead(pin) != (state)) {                                                                               \
		if(esp_get_ccount() - start_cycle_count > timeout_cycles) {                                                    \
			return 0;                                                                                                  \
		}                                                                                                              \
	}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
	const uint32_t max_timeout_us = clockCyclesToMicroseconds(UINT_MAX);
	if(timeout > max_timeout_us) {
		timeout = max_timeout_us;
	}
	const uint32_t timeout_cycles = microsecondsToClockCycles(timeout);
	const uint32_t start_cycle_count = esp_get_ccount();
	WAIT_FOR_PIN_STATE(!state);
	WAIT_FOR_PIN_STATE(state);
	const uint32_t pulse_start_cycle_count = esp_get_ccount();
	WAIT_FOR_PIN_STATE(!state);
	return clockCyclesToMicroseconds(esp_get_ccount() - pulse_start_cycle_count);
}
