/*
 * File: HardwarePWM.h
 * Original Author: https://github.com/hrsavla
 *
 * This HW_PWM library enables Sming framework user to use ESP SDK PWM API
 * Period of PWM is fixed to 1000ms / Frequency = 1khz
 * Duty at 100% = 22222. Duty at 0% = 0
 * You can use function setPeriod() to change frequency/period.
 * Calculate the Duty as per the formulae give in ESP8266 SDK
 * Duty = (Period *1000)/45
 *
 * PWM can be generated on upto 8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 */
/** @defgroup   hw_pwm Hardware PWM functions
 *  @brief      Provides hardware pulse width modulation functions
*/

#ifndef HARDWAREPWM_H
#define	HARDWAREPWM_H

#include "ESP8266EX.h"
#include "../Wiring/WiringFrameworkDependencies.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <pwm.h>
#ifdef __cplusplus
}
#endif

#define PWM_BAD_CHANNEL 0xff

class HardwarePWM {
public:
    /** @brief  Instantiate hardware PWM object
     *  @param  pins Pointer to array of pins to control
     *  @param  no_of_pins Quantity of elements in array of pins
     *  @addtogroup hw_pwm
     *  @{
     */
	HardwarePWM(uint8 *pins, uint8 no_of_pins);
	virtual ~HardwarePWM();

    /** @brief  Set PWM duty cycle
     *  @param  pin GPIO to set
     *  @param  duty Value of duty cycle to set pin to
     *  @retval bool True on success
     */
	bool analogWrite(uint8 pin, uint32 duty);

    /** @brief  Set PWM duty cycle
     *  @param  pin GPIO to set
     *  @param  duty Value of duty cycle to set pin to
     *  @retval bool True on success
     */
	bool setDuty(uint8 pin, uint32 duty);

    /** @brief  Get PWM duty cycle
     *  @param  pin GPIO to get duty cycle for
     *  @retval uint32 Value of PWM duty cycle
     */
	uint32 getDuty(uint8 pin);

    /** @brief  Set PWM period
     *  @param  period PWM period
     *  @note   All PWM pins share the same period
     */
	void setPeriod(uint32 period);

    /** @brief  Get PWM period
     *  @retval uint32 Value of PWM period
     */
	uint32 getPeriod(void);

    /** @brief  Get channel number for a pin
     *  @param  pin GPIO to interrogate
     *  @retval uint8 Channel of GPIO
     */
	uint8 getChannel(uint8 pin);

    /** @brief  Get the maximum duty cycle value
     *  @retval uint32 Maximum permissible duty cycle
     *  @note   Attempt to set duty of a pin above this value will fail
     */
	uint32 getMaxDuty();
	/** @} */
protected:

private:
	uint8 channel_count;
	uint8 channels[PWM_CHANNEL_NUM_MAX];
	uint32 maxduty;
};

#endif	/* HARDWAREPWM_H */
