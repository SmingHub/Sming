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

#define GP_IN(pin) ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> (pin)) & 1)
#define GP_OUT(pin, val) GPIO_REG_WRITE((GPIO_OUT_W1TC_ADDRESS >> ((val)&1)), BIT(pin))

#ifdef SPISOFT_DELAY_VARIABLE
uint8_t checkSpeed(SPISpeed& speed);
#endif
