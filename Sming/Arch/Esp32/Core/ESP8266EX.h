/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ESP8266EX.h
 *
 ****/

#pragma once

#include "user_config.h"

#define TOTAL_PINS 16
#define NUM_DIGITAL_PINS TOTAL_PINS

/** @brief  ESP GPIO pin configuration
 *  @ingroup constants
 */
struct EspDigitalPin {
	uint8_t id;
	uint32_t mux;
	uint8_t gpioFunc;

	operator const int() const
	{
		return id;
	}
	void mode(uint8_t mode) const;
	void write(uint8_t val) const;
	uint8_t read() const;
};

/** @brief  ESP GPIO pin configuration
 *  @ingroup gpio
 */
extern const EspDigitalPin EspDigitalPins[];
