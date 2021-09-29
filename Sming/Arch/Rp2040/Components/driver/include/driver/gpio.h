/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gpio.h
 *
 ****/

#pragma once

#include <hardware/gpio.h>

/**
 * @defgroup gpio_driver GPIO driver
 * @ingroup drivers
 * @{
 */

enum GPIO_INT_TYPE {
	GPIO_PIN_INTR_DISABLE = 0,
	GPIO_PIN_INTR_POSEDGE = GPIO_IRQ_EDGE_RISE,
	GPIO_PIN_INTR_NEGEDGE = GPIO_IRQ_EDGE_FALL,
	GPIO_PIN_INTR_ANYEDGE = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
	GPIO_PIN_INTR_LOLEVEL = GPIO_IRQ_LEVEL_LOW,
	GPIO_PIN_INTR_HILEVEL = GPIO_IRQ_LEVEL_HIGH,
};
