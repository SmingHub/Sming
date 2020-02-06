/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gpio.h
 *
 ****/

/** @file gpio.h */

#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <gpio.h>

/**
 * @defgroup gpio_driver GPIO driver
 * @ingroup drivers
 * @{
 */

/**	@enum	GPIO_INT_TYPE
	@brief	Defines the GPIO interrupt type

	@var	GPIO_INT_TYPE::GPIO_PIN_INTR_DISABLE
			Interrupt disabled for this pin
	@var	GPIO_INT_TYPE::GPIO_PIN_INTR_POSEDGE
			Interrupt occurs on positive edge
	@var	GPIO_INT_TYPE::GPIO_PIN_INTR_NEGEDGE
			Interrupt occurs on negative edge
	@var	GPIO_INT_TYPE::GPIO_PIN_INTR_ANYEDGE
			Interrupt occurs on both positive and negative edge
	@var	GPIO_INT_TYPE::GPIO_PIN_INTR_LOLEVEL
			Interrupt occurs when GPIO low
	@var	GPIO_INT_TYPE::GPIO_PIN_INTR_HILEVEL
			Interrupt occurs when GPIO high
 */

/** @} */

#ifdef __cplusplus
}
#endif
