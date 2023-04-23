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
 * PWM can be generated on up to 8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 *
 * See also ESP8266 Technical Reference, Chapter 12:
 * http://espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf
 *
 */

#include <HardwarePWM.h>
#include <Clock.h>
#include "ESP8266EX.h"
#include <debug_progmem.h>

#define PERIOD_TO_MAX_DUTY(x) (x * 25)

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t noOfPins) : channel_count(noOfPins)
{
	if(noOfPins == 0) {
		return;
	}

	uint32_t ioInfo[PWM_CHANNEL_NUM_MAX][3];   // pin information
	uint32_t pwmDutyInit[PWM_CHANNEL_NUM_MAX]; // pwm duty
	for(uint8_t i = 0; i < noOfPins; i++) {
		ioInfo[i][0] = EspDigitalPins[pins[i]].mux;
		ioInfo[i][1] = EspDigitalPins[pins[i]].gpioFunc;
		ioInfo[i][2] = EspDigitalPins[pins[i]].id;
		pwmDutyInit[i] = 0; // Start with zero output
		channels[i] = pins[i];
	}
	const int initial_period = 1000;
	pwm_init(initial_period, pwmDutyInit, noOfPins, ioInfo);
	update();
	maxduty = PERIOD_TO_MAX_DUTY(initial_period); // for period of 1000
}

HardwarePWM::~HardwarePWM()
{
	// There is no function in the SDK to stop PWM output, yet.
}

uint8_t HardwarePWM::getChannel(uint8_t pin)
{
	for(uint8_t i = 0; i < channel_count; i++) {
		if(channels[i] == pin) {
			return i;
		}
	}

	debug_d("getChannel: can't find pin %d", pin);
	return PWM_BAD_CHANNEL;
}

uint32_t HardwarePWM::getDutyChan(uint8_t chan)
{
	return (chan == PWM_BAD_CHANNEL) ? 0 : pwm_get_duty(chan);
}

bool HardwarePWM::setDutyChan(uint8_t chan, uint32_t duty, bool update)
{
	if(chan == PWM_BAD_CHANNEL) {
		return false;
	}

	if(duty <= maxduty) {
		pwm_set_duty(duty, chan);
		if(update) {
			this->update();
		}
		return true;
	}

	debug_e("Duty cycle value too high for current period. max duty is %d.",maxduty);
	return false;
}

uint32_t HardwarePWM::getPeriod()
{
	return pwm_get_period();
}

void HardwarePWM::setPeriod(uint32_t period)
{
	maxduty = PERIOD_TO_MAX_DUTY(period);
	pwm_set_period(period);
	update();
}

void HardwarePWM::update()
{
	pwm_start();
}

uint32_t HardwarePWM::getFrequency(uint8_t pin)
{
	(void)pin;
	auto period = pwm_get_period();
	return (period == 0) ? 0 : 1000000U / period;
}
