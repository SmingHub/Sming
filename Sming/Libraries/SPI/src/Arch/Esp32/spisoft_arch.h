/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * softspi_arch.h - Esp32
 *
 */

#pragma once

#include <hal/gpio_ll.h>

#define GP_IN(pin) gpio_ll_get_level(&GPIO, gpio_num_t(pin))
#define GP_OUT(pin, val) gpio_ll_set_level(&GPIO, gpio_num_t(pin), (val)&1)
