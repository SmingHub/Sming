/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwarePWM.h
 *
 * Original Author: https://github.com/hrsavla
 *
 * This HW_PWM library enables Sming framework user to use ESP SDK PWM API
 * Period of PWM is fixed to 1000ms / Frequency = 1khz
 * Duty at 100% = 22222. Duty at 0% = 0
 * You can use function setPeriod() to change frequency/period.
 * Calculate the Duty as per the formulae give in ESP8266 SDK
 * Duty = (Period *1000)/45
 *
 * PWM can be generated on up to 8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 *
 ****/

/** @defgroup   hw_pwm Hardware PWM functions
 *  @brief      Provides hardware pulse width modulation functions
 *  @{
*/

#pragma once

#include <cstdint>
#include <driver/pwm.h>

#define PWM_BAD_CHANNEL 0xff ///< Invalid PWM channel

/**
 * @brief Basic pulse width modulation support.
 *
 * PERIOD: A full PWM cycle has a duration, measured in hardware timer counts
 * DUTY: The 'ON' portion of a PWM cycle measured in hardware timer counts
 *
 * These values differ between architectures.
 * 
 * DUTY is always <= PERIOD.
 * If DUTY == 0 then output is OFF.
 * If DUTY == PERIOD then output is ON.
 * Intermediate values produce a proportional output, PERCENT = 100 * DUTY / PERIOD.
 *
 * Producing an analogue voltage from such a PWM output requires a suitable filter
 * with characteristics tuned to the specific PWM frequency in use.
 * A single-pole R/C filter with R*C = PERIOD (in seconds) is often sufficient.
 *
 * Note: Devices such as servo motors use the digital signal directly.
 *
 * Note: The Esp8266 has no PWM hardware and uses an interrupt-based approach (software PWM).
 *
 * Note: Esp32 and Rp2040 hardware offers more capability which requires use of SDK calls
 * or direct hardware access. To avoid resource conflicts avoid using this class in those cases.
 */
class HardwarePWM
{
public:
	/** @brief  Instantiate hardware PWM object
     *  @param  pins Pointer to array of pins to control
     *  @param  no_of_pins Quantity of elements in array of pins
     */
	HardwarePWM(uint8_t* pins, uint8_t no_of_pins);

	virtual ~HardwarePWM();

	/** @brief  Set PWM duty cycle
     *  @param  pin GPIO to set
     *  @param  duty Value of duty cycle to set pin to
     *  @retval bool True on success
     *  @note   Default frequency is 1khz but can be varied by various function
     */
	bool analogWrite(uint8_t pin, uint32_t duty)
	{
		return setDuty(pin, duty);
	}

	/** @brief  Set PWM duty cycle for a channel
     *  @param  chan Channel to set
     *  @param  duty Value of duty cycle to set channel to
     *  @param  update Update PWM output
     *  @retval bool True on success
     */
	bool setDutyChan(uint8_t chan, uint32_t duty, bool update = true);

	/** @brief  Set PWM duty cycle
     *  @param  pin GPIO to set
     *  @param  duty Value of duty cycle (timer ticks) to set pin to
     *  @param  update Update PWM output
     *  @retval bool True on success
     *  @note   This function is used to set the pwm duty cycle for a given pin. If parameter 'update' is false
     *        	then you have to call update() later to update duties.
     */
	bool setDuty(uint8_t pin, uint32_t duty, bool update = true)
	{
		uint8_t chan = getChannel(pin);
		return setDutyChan(chan, duty, update);
	}

	/** @brief  Get PWM duty cycle
	 *  @param  chan Channel to get duty cycle for
	 *  @retval uint32_t Value of PWM duty cycle in timer ticks
	 */
	uint32_t getDutyChan(uint8_t chan);

	/** @brief  Get PWM duty cycle
     *  @param  pin GPIO to get duty cycle for
     *  @retval uint32_t Value of PWM duty cycle in timer ticks
     */
	uint32_t getDuty(uint8_t pin)
	{
		uint8_t chan = getChannel(pin);
		return getDutyChan(chan);
	}

	/** @brief  Set PWM period
     *  @param  period PWM period in microseconds
     *  @note   All PWM pins share the same period
     */
	void setPeriod(uint32_t period);

	/** @brief  Get PWM period
     *  @retval uint32_t Value of PWM period in microseconds
     */
	uint32_t getPeriod();

	/** @brief  Get channel number for a pin
     *  @param  pin GPIO to interrogate
     *  @retval uint8_t Channel of GPIO
     */
	uint8_t getChannel(uint8_t pin) const;

	/** @brief  Get the maximum duty cycle value
     *  @retval uint32_t Maximum permissible duty cycle in timer ticks
     *  @note   Attempt to set duty of a pin above this value will fail
     */
	uint32_t getMaxDuty()
	{
		return maxduty;
	}

	/** @brief  This function is used to actually update the PWM.
	 */
	void update();

	/** @brief Get PWM Frequency
	 *  @param pin GPIO to get frequency for
	 *  @retval uint32_t Value of Frequency 
	*/
	uint32_t getFrequency(uint8_t pin);

private:
	uint8_t channel_count;
	uint8_t channels[PWM_CHANNEL_NUM_MAX];
	uint32_t maxduty{0};
};

/** @} */
