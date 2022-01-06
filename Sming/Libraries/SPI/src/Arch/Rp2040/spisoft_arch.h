/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * softspi_arch.h - Rp2040
 *
 */

#pragma once

#include <hardware/gpio.h>

#define GP_IN(pin) gpio_get(pin)
#define GP_OUT(pin, val) gpio_put(pin, (val)&1)
