/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DigitalHooks.h - Support for hooking digital functions
 *
 ****/

#pragma once

#include <stdint.h>

/** @brief Class to customise behaviour for digital functions
 *  @note By default, actions get output to console but this can get very busy.
 *  The easiest way to change the behaviour is by sub-classing DigitalHooks
 *  and passing the new class instance to `setDigitalHooks`.
 */
class DigitalHooks
{
public:
	/**
	 * @brief Report invalid pin number
	 * @param function Name of function which was called
	 * @param pin The pin number
	 */
	virtual void badPin(const char* function, uint16_t pin);

	/**
	 * @brief Set pin mode
	 * @param pin Has already been range checked
	 * @param mode
   * @retval true if mode can be set for this pin, will be stored
	 */
	virtual bool pinMode(uint16_t pin, uint8_t mode);

	/**
   * @brief Change pin output
   * @param pin Has already been range checked
   * @param val New pin value
   */
	virtual void digitalWrite(uint16_t pin, uint8_t val);

	/**
   * @brief Read pin state
   * @param pin Has already been range checked
   * @param mode The currently set mode for this pin
   * @param val State for pin
   */
	virtual uint8_t digitalRead(uint16_t pin, uint8_t mode);

	/**
   * @brief Set or clear pullup state for a pin
   * @param pin Has already been range checked
   * @param enable true for pullup, false for no pullup
   */
	virtual void pullup(uint16_t pin, bool enable);

	/** @brief Measure duration of pulse on GPIO
	 *  @param pin GPIO to measure
	 *  @param state State of pulse to measure [HIGH | LOW]
	 *  @param timeout Maximum duration of pulse
	 *  @retval unsigned long Pulse duration in microseconds
	 */
	virtual unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout);
};
