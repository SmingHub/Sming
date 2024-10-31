/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Rp2040/Core/HardwarePWM.cpp
 *
 */

#include <HardwarePWM.h>

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t noOfPins) : channel_count(noOfPins)
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

uint32_t HardwarePWM::getFrequency(uint8_t pin)
{
	return 0;
}
