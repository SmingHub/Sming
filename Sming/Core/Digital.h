/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.h
 *
 ****/

/** @defgroup   gpio GPIO functions
 *  @brief      Provides general purpose input and output (GPIO) functions
 *  @see        pwm
 *  @{
*/

#pragma once

#include "WiringFrameworkDependencies.h"

/** @brief  Set the mode of a GPIO pin
 *  @param  pin GPIO pin to configure
 *  @param  mode Mode of pin [INPUT | INPUT_PULLUP | OUTPUT]
 *  @todo   ESP8266 supports pull-down on GPIO 0-15
 */
void pinMode(uint16_t pin, uint8_t mode);

/** @brief  Set a digital output
 *  @param  pin GPIO pin to set
 *  @param  val Value to set [0 | 1]
 */
void IRAM_ATTR digitalWrite(uint16_t pin, uint8_t val);

/** @brief  Get the value of a digital input
 *  @param  pin GPIO pin to read
 *  @retval uint8_t Value of GPIO
 */
uint8_t IRAM_ATTR digitalRead(uint16_t pin);

/** @brief  Enable pull-up on digital input
 *  @param  pin GPIO to enable pull-up resistor
 */
void IRAM_ATTR pullup(uint16_t pin);

/** @brief  Disable pull-up on digital input
 *  @param  pin GPIO to disable pull-up resistor
 */
void IRAM_ATTR noPullup(uint16_t pin);

/** @brief  Check if GPIO is an input
 *  @param  pin GPIO to check
 *  @retval bool True if GPIO is an input
 */
bool IRAM_ATTR isInputPin(uint16_t pin);

/** @brief  Measures duration of pulse on GPIO
 *  @param  pin GPIO to measure
 *  @param  state State of pulse to measure [HIGH | LOW]
 *  @param  timeout Maximum duration of pulse
 *  @retval "unsigned long" Pulse duration in microseconds
 *  @note   Works on pulses from 2-3 microseconds to 3 minutes in length.
 *  @note   Must be called at least a few dozen microseconds before the start of the pulse.
 */
unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout = 1000000L);

inline uint16_t analogRead(uint16_t pin)
{
	if(pin == A0)
		return system_adc_read();
	else
		return -1; // Not supported
}

/** @} */
