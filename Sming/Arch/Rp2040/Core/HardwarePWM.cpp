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
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include <algorithm>

#define PWM_REF_CLOCK 125e6
#define PWM_CLOCKDIV 100
#define PWM_FREQ 1000
#define PWM_CLOCK (PWM_REF_CLOCK / PWM_CLOCKDIV)
#define PWM_COUNT_TOP (uint32_t((PWM_CLOCK / PWM_FREQ) + 0.5) + 1)

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t noOfPins) : channel_count(noOfPins)
{
	assert(noOfPins > 0 && noOfPins <= PWM_CHANNEL_NUM_MAX);
	noOfPins = std::min(uint8_t(PWM_CHANNEL_NUM_MAX), noOfPins);

	for(uint8_t i = 0; i < noOfPins; i++) {
		auto pwmPin = pins[noOfPins];
		channels[i] = pwmPin;

		auto slice_num = pwm_gpio_to_slice_num(pwmPin);
		pwm_config cfg = pwm_get_default_config();
		pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_FREE_RUNNING);
		pwm_config_set_clkdiv(&cfg, PWM_CLOCKDIV);
		pwm_config_set_wrap(&cfg, PWM_COUNT_TOP);
		pwm_init(slice_num, &cfg, true);
		gpio_set_function(pwmPin, GPIO_FUNC_PWM);
		gpio_set_dir(pwmPin, GPIO_OUT);
		pwm_set_gpio_level(pwmPin, 0);
	}
	maxduty = maxDuty(DEFAULT_RESOLUTION);
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
