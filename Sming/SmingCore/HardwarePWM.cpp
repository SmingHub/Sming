/*
 * File: HardwarePWM.cpp
 * Original Author: https://github.com/hrsavla
 *
 * This HardwarePWM library enables Sming framework user to use ESP SDK PWM API
 * Period of PWM is fixed to 1000ms / Frequency = 1khz
 * Duty at 100% = 22222. Duty at 0% = 0
 * You can use function setPeriod() to change frequency/period.
 * Calculate the Duty as per the formulae give in ESP8266 SDK
 * Duty = (Period *1000)/45
 *
 * PWM can be generated on upto 8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 */

#include "Clock.h"
#include "../Wiring/WiringFrameworkIncludes.h"

#include "HardwarePWM.h"

HardwarePWM::HardwarePWM(uint8 *pins, uint8 no_of_pins) {
	channel_count = no_of_pins;
	if (no_of_pins > 0) {
		uint32 io_info[PWM_CHANNEL_NUM_MAX][3]; // pin information
		uint32 pwm_duty_init[PWM_CHANNEL_NUM_MAX]; // pwm duty
		for (uint8 i = 0; i < no_of_pins; i++) {
			io_info[i][0] = EspDigitalPins[pins[i]].mux;
			io_info[i][1] = EspDigitalPins[pins[i]].gpioFunc;
			io_info[i][2] = EspDigitalPins[pins[i]].id;
			pwm_duty_init[i] = 0; // Start with zero output
			channels[i] = pins[i];
		}
		pwm_init(1000, pwm_duty_init, no_of_pins, io_info);
		pwm_start();
	}
}

HardwarePWM::~HardwarePWM() {
	// There is no function in the SDK to stop PWM output, yet.
}

/* Function Name: getChannel
 * Description: This function is used to get channel number for given pin
 * Parameters: pin - Esp8266 pin number
 */
uint8 HardwarePWM::getChannel(uint8 pin) {
	for (uint8 i = 0; i < channel_count; i++) {
		if (channels[i] == pin) {
			//debugf("getChannel %d is %d", pin, i);
			return i;
		}
	}
	//debugf("getChannel: can't find pin %d", pin);
	return PWM_BAD_CHANNEL;
}

/* Function Name: analogWrite
 * Description: This function is used to set the pwm duty cycle for a given pin
 * Parameters: pin - Esp8266 pin number
 *             duty - duty cycle value (valid values are 0-22222)
 * Default frequency is 1khz but can be varied by various function
 */
void HardwarePWM::analogWrite(uint8 pin, uint32 duty) {
	setDuty(pin, duty);
}

/* Function Name: getDuty
 * Description: This function is used to get the duty cycle number for a given pin
 * Parameters: pin -Esp8266 pin number
 */
uint32 HardwarePWM::getDuty(uint8 pin) {
	uint8 chan = getChannel(pin);
	if (chan == PWM_BAD_CHANNEL) return 0;
	else return pwm_get_duty(chan);
}

/* Function Name: setDuty
 * Description: This function is used to set the pwm duty cycle for a given pin
 * Parameters: pin - pin number
 *             duty - duty cycle value (valid values are 0-22222)
 */
void HardwarePWM::setDuty(uint8 pin, uint32 duty) {
	uint8 chan = getChannel(pin);
	if (chan == PWM_BAD_CHANNEL) return;
	if (duty <= 22222) {
		pwm_set_duty(duty, chan);
		pwm_start();
		delay(25); // delay is needed otherwise PWM is not set
	} else {
		debugf("Duty cycle value too high. Should be less than 22222.");
	}
}

/* Function Name: getPeriod
 * Description: This function is used to get Period of PWM.
 *				Period / frequency will remain same for all pins.
 *
 */
uint32 HardwarePWM::getPeriod() {
	return pwm_get_period();
}

/* Function Name: setPeriod
 * Description: This function is used to set Period of PWM.
 *				Period / frequency will remain same for all pins.
 */
void HardwarePWM::setPeriod(uint32 period) {
	pwm_set_period(period);
	pwm_start();
}
