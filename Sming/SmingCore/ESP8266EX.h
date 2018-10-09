/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_ESP8266EX_H_
#define _SMING_CORE_ESP8266EX_H_

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

	operator const int()
	{
		return id;
	}
	void mode(uint8_t mode);
	void write(uint8_t val);
	uint8_t read();
};

/** @brief  ESP GPIO pin configuration
 *  @ingroup gpio
 */
extern EspDigitalPin EspDigitalPins[];

#endif /* _SMING_CORE_ESP8266EX_H_ */
