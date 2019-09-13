/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Interrupts.h
 *
 ****/

/** @defgroup   interrupts Interrupt functions
 *  @brief      Provides interrupt functions
 *  @{
*/
#pragma once

#include <stdint.h>
#include <driver/gpio.h>
#include <Delegate.h>
#include <WConstants.h>
#include <sming_attr.h>

constexpr unsigned ESP_MAX_INTERRUPTS = 16;

typedef void (*InterruptCallback)();
typedef Delegate<void()> InterruptDelegate;

/** @brief  Convert Arduino interrupt mode to Sming mode
 *  @param  mode Arduino mode type
 *  @retval GPIO_INT_TYPE Sming interrupt type
 */
__forceinline GPIO_INT_TYPE ConvertArduinoInterruptMode(uint8_t mode)
{
	switch(mode) {
	case LOW: // to trigger the interrupt whenever the pin is low,
		return GPIO_PIN_INTR_LOLEVEL;
	case CHANGE:				 // to trigger the interrupt whenever the pin changes value
		return (GPIO_INT_TYPE)3; // GPIO_PIN_INTR_ANYEDGE
	case RISING:				 // to trigger when the pin goes from low to high,
		return GPIO_PIN_INTR_POSEDGE;
	case FALLING: // for when the pin goes from high to low.
		return GPIO_PIN_INTR_NEGEDGE;
	case HIGH: // to trigger the interrupt whenever the pin is high.
		return GPIO_PIN_INTR_HILEVEL;
	default:
		return GPIO_PIN_INTR_DISABLE;
	}
}

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  callback Function to call when interrupt occurs on GPIO
 *  @param  type Interrupt type
 *  @note   Traditional c-type callback function method
 */
void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE type);

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  callback Function to call when interrupt occurs on GPIO
 *  @param  mode Arduino type interrupt mode
 *  @note   Traditional c-type callback function method, MUST use IRAM_ATTR
 *  Use this type of interrupt handler for timing-sensitive applications.
 */
__forceinline void attachInterrupt(uint8_t pin, InterruptCallback callback, uint8_t mode)
{
	GPIO_INT_TYPE type = ConvertArduinoInterruptMode(mode);
	attachInterrupt(pin, callback, type);
}

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  delegateFunction Function to call when interrupt occurs on GPIO
 *  @param  type Interrupt type
 *  @note   Delegate function method
 */
void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, GPIO_INT_TYPE type);

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  delegateFunction Function to call when interrupt occurs on GPIO
 *  @param  mode Arduino type interrupt mode (LOW, HIGH, CHANGE, RISING, FALLING)
 *  @note   Delegate function method, can be a regular function, method, etc.
 *  The delegate function is called via the system task queue so does not need any special consideration.
 *  Note that this type of interrupt handler is not suitable for timing-sensitive applications.
 */
__forceinline void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, uint8_t mode)
{
	GPIO_INT_TYPE type = ConvertArduinoInterruptMode(mode);
	attachInterrupt(pin, delegateFunction, type);
}

/** @brief  Enable interrupts on GPIO pin
 *  @param  pin GPIO to enable interrupts for
 *  @param  type Interrupt type
 *  @note   Configure interrupt handler with attachInterrupt(pin, callback, type)
 */
void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE type);

/** @brief  Disable interrupts on GPIO pin
 *  @param  pin GPIO to disable interrupts for
 */
void detachInterrupt(uint8_t pin);

/** @brief  Set interrupt mode
 *  @param  pin GPIO to configure
 *  @param  type Interrupt type
 *  @note   Use ConvertArduinoInterruptMode to get Sming interrupt type from an Arduino interrupt type
 */
void interruptMode(uint8_t pin, GPIO_INT_TYPE type);

/** @brief  Set interrupt mode
 *  @param  pin GPIO to configure
 *  @param  mode Interrupt mode
 *  @note   Use ConvertArduinoInterruptMode to get Sming interrupt type from an Arduino interrupt type
 */
__forceinline void interruptMode(uint8_t pin, uint8_t mode)
{
	GPIO_INT_TYPE type = ConvertArduinoInterruptMode(mode);
	interruptMode(pin, type);
}

#define digitalPinToInterrupt(pin) ((pin) < ESP_MAX_INTERRUPTS ? (pin) : -1)

// AVR-style interrupt management
#define cli() noInterrupts()
#define sei() interrupts()

/** @} */
