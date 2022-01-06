/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * softspi_arch.h - Esp8266
 *
 */

#pragma once

#include <fast_io.h>

#define GP_IN(pin) GP_FAST_READ(pin, PERIPHS_GPIO_BASEADDR + GPIO_IN_ADDRESS)
#define GP_OUT(pin, val)                                                                                               \
	GP_FAST_WRITE(pin, val, PERIPHS_GPIO_BASEADDR + GPIO_OUT_W1TC_ADDRESS,                                             \
				  PERIPHS_GPIO_BASEADDR + GPIO_OUT_W1TS_ADDRESS)

namespace spisoft
{
constexpr SpiPins defaultPins{
	.sck = 14,
	.miso = 12,
	.mosi = 13,
};

#ifdef SPISOFT_DELAY_VARIABLE
uint8_t checkSpeed(SPISpeed& speed);
#endif

} // namespace spisoft
