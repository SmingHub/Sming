/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * pwm.h
 *
 ****/


#pragma once

#ifdef SOC_LEDC_CHANNEL_NUM
#define PWM_CHANNEL_NUM_MAX SOC_LEDC_CHANNEL_NUM
#else
// this should not happen if the correct esp32 includes are used, just to be absolutely sure
#define PWM_CHANNEL_NUM_MAX 8
#endif
