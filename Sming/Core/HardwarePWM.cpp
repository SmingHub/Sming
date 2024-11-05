/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwarePWM.cpp
 *
 * Common code for all architectures
 *
 */

#include "HardwarePWM.h"
#include <debug_progmem.h>

uint8_t HardwarePWM::getChannel(uint8_t pin) const
{
	for(unsigned i = 0; i < channel_count; ++i) {
		if(channels[i] == pin) {
			return i;
		}
	}

	debug_d("[HWPWM] getChannel: can't find pin %u", pin);
	return PWM_BAD_CHANNEL;
}
