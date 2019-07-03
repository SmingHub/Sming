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

#include "WiringFrameworkDependencies.h"

#define ESP_MAX_INTERRUPTS 16

typedef void (*InterruptCallback)();
typedef Delegate<void()> InterruptDelegate;

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  callback Function to call when interrupt occurs on GPIO
 *  @param  mode Arduino type interrupt mode
 *  @note   Traditional c-type callback function method, MUST use IRAM_ATTR
 *  Use this type of interrupt handler for timing-sensitive applications.
 */
void attachInterrupt(uint8_t pin, InterruptCallback callback, uint8_t mode);

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  delegateFunction Function to call when interrupt occurs on GPIO
 *  @param  mode Arduino type interrupt mode
 *  @note   Delegate function method, can be a regular function, method, etc.
 *  The delegate function is called via the system task queue so does not need any special consideration.
 *  Note that this type of interrupt handler is not suitable for timing-sensitive applications.
 */
void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, uint8_t mode);

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  callback Function to call when interrupt occurs on GPIO
 *  @param  mode Interrupt mode
 *  @note   Traditional c-type callback function method
 *  @todo   Add GPIO_INT_TYPE documentation - is this in SDK?
 */
void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE mode); // ESP compatible version

/** @brief  Attach a function to a GPIO interrupt
 *  @param  pin GPIO to configure
 *  @param  delegateFunction Function to call when interrupt occurs on GPIO
 *  @param  mode Interrupt mode
 *  @note   Delegate function method
 */
void attachInterrupt(uint8_t pin, InterruptDelegate delegateFunction, GPIO_INT_TYPE mode); // ESP compatible version

/** @brief  Enable interrupts on GPIO pin
 *  @param  pin GPIO to enable interrupts for
 *  @param  mode Interrupt mode
 *  @note   Configure interrupt handler with attachInterrupt(pin, callback, mode)
 */
void attachInterruptHandler(uint8_t pin, GPIO_INT_TYPE mode);

/** @brief  Disable interrupts on GPIO pin
 *  @param  pin GPIO to disable interrupts for
 */
void detachInterrupt(uint8_t pin);

/** @brief  Set interrupt mode
 *  @param  pin GPIO to configure
 *  @param  mode Interrupt mode
 *  @note   Use ConvertArduinoInterruptMode to get Sming interrupt type from an Arduino interrupt type
 */
void interruptMode(uint8_t pin, uint8_t mode);

/** @brief  Set interrupt mode
 *  @param  pin GPIO to configure
 *  @param  type Interrupt type
 *  @note   Use ConvertArduinoInterruptMode to get Sming interrupt type from an Arduino interrupt type
 */
void interruptMode(uint8_t pin, GPIO_INT_TYPE type);

/** @brief  Convert Arduino interrupt mode to Sming mode
 *  @param  mode Arduino mode type
 *  @retval GPIO_INT_TYPE Sming interrupt mode type
 */
GPIO_INT_TYPE ConvertArduinoInterruptMode(uint8_t mode);

#define digitalPinToInterrupt(pin) ((pin) < ESP_MAX_INTERRUPTS ? (pin) : -1)

// AVR-style interrupt management
#define cli() noInterrupts()
#define sei() interrupts()

/** @} */
