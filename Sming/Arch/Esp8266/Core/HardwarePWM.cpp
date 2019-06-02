/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwarePWM.cpp
 *
 * Original Author: https://github.com/hrsavla
 *
 * This HardwarePWM library enables Sming framework user to use ESP SDK PWM API
 * Period of PWM is fixed to 1000us / Frequency = 1khz
 * Duty at 100% = 22222. Duty at 0% = 0
 * You can use function setPeriod() to change frequency/period.
 * Calculate the max duty as per the formulae give in ESP8266 SDK
 * Max Duty = (Period * 1000) / 45
 *
 * PWM can be generated on upto 8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 *
 * See also ESP8266 Technical Reference, Chapter 12:
 * http://espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf
 *
 */

#include "Clock.h"
#include "WiringFrameworkIncludes.h"
#include "ESP8266EX.h"

#include "HardwarePWM.h"

#define PERIOD_TO_MAX_DUTY(x) (x * 25)

HardwarePWM::HardwarePWM(uint8* pins, uint8 no_of_pins) : channel_count(no_of_pins)
{
	if(no_of_pins > 0) {
		uint32 io_info[PWM_CHANNEL_NUM_MAX][3];	// pin information
		uint32 pwm_duty_init[PWM_CHANNEL_NUM_MAX]; // pwm duty
		for(uint8 i = 0; i < no_of_pins; i++) {
			io_info[i][0] = EspDigitalPins[pins[i]].mux;
			io_info[i][1] = EspDigitalPins[pins[i]].gpioFunc;
			io_info[i][2] = EspDigitalPins[pins[i]].id;
			pwm_duty_init[i] = 0; // Start with zero output
			channels[i] = pins[i];
		}
		const int initial_period = 1000;
		pwm_init(initial_period, pwm_duty_init, no_of_pins, io_info);
		update();
		maxduty = PERIOD_TO_MAX_DUTY(initial_period); // for period of 1000
	}
}

HardwarePWM::~HardwarePWM()
{
	// There is no function in the SDK to stop PWM output, yet.
}

/* Function Name: getChannel
 * Description: This function is used to get channel number for given pin
 * Parameters: pin - Esp8266 pin number
 */
uint8 HardwarePWM::getChannel(uint8 pin)
{
	for(uint8 i = 0; i < channel_count; i++) {
		if(channels[i] == pin) {
			//debugf("getChannel %d is %d", pin, i);
			return i;
		}
	}
	//debugf("getChannel: can't find pin %d", pin);
	return PWM_BAD_CHANNEL;
}

/* Function Name: getDutyChan
 * Description: This function is used to get the duty cycle number for a given channel
 * Parameters: chan -Esp8266 channel number
 */
uint32 HardwarePWM::getDutyChan(uint8 chan)
{
	if(chan == PWM_BAD_CHANNEL) {
		return 0;
	} else {
		return pwm_get_duty(chan);
	}
}

/* Function Name: setDutyChan
 * Description: This function is used to set the pwm duty cycle for a given channel. If parameter 'update' is false
 *              then you have to call update() later to update duties.
 * Parameters: chan - channel number
 *             duty - duty cycle value
 *             update - update PWM output
 */
bool HardwarePWM::setDutyChan(uint8 chan, uint32 duty, bool update)
{
	if(chan == PWM_BAD_CHANNEL) {
		return false;
	} else if(duty <= maxduty) {
		pwm_set_duty(duty, chan);
		if(update) {
			this->update();
		}
		return true;
	} else {
		debugf("Duty cycle value too high for current period.");
		return false;
	}
}

/* Function Name: getPeriod
 * Description: This function is used to get Period of PWM.
 *				Period / frequency will remain same for all pins.
 *
 */
uint32 HardwarePWM::getPeriod()
{
	return pwm_get_period();
}

/* Function Name: setPeriod
 * Description: This function is used to set Period of PWM.
 *				Period / frequency will remain same for all pins.
 */
void HardwarePWM::setPeriod(uint32 period)
{
	maxduty = PERIOD_TO_MAX_DUTY(period);
	pwm_set_period(period);
	update();
}

/* Function Name: update
 * Description: This function is used to actually update the PWM.
 */
void HardwarePWM::update()
{
	pwm_start();
}
