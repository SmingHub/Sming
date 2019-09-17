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

#include <HardwarePWM.h>

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t no_of_pins) : channel_count(no_of_pins)
{
}

HardwarePWM::~HardwarePWM()
{
}

uint8_t HardwarePWM::getChannel(uint8_t pin)
{
	return PWM_BAD_CHANNEL;
}

uint32_t HardwarePWM::getDutyChan(uint8_t chan)
{
	return 0;
}

bool HardwarePWM::setDutyChan(uint8_t chan, uint32_t duty, bool update)
{
	return false;
}

uint32_t HardwarePWM::getPeriod()
{
	return 0;
}

void HardwarePWM::setPeriod(uint32_t period)
{
}

void HardwarePWM::update()
{
}
