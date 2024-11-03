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

static_assert(PWM_CHANNEL_NUM_MAX == 16);

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t noOfPins) : channel_count(noOfPins)
{
	assert(noOfPins > 0 && noOfPins <= PWM_CHANNEL_NUM_MAX);
	noOfPins = std::min(uint8_t(PWM_CHANNEL_NUM_MAX), noOfPins);

	for(uint8_t i = 0; i < noOfPins; i++) {
		auto pin = pins[i];
		channels[i] = pin;

		auto slice_num = pwm_gpio_to_slice_num(pin);
		pwm_config cfg = pwm_get_default_config();
		pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_FREE_RUNNING);
		pwm_config_set_clkdiv(&cfg, PWM_CLOCKDIV);
		pwm_config_set_wrap(&cfg, PWM_COUNT_TOP);
		pwm_init(slice_num, &cfg, true);
		gpio_set_function(pin, GPIO_FUNC_PWM);
		gpio_set_dir(pin, GPIO_OUT);
		pwm_set_gpio_level(pin, 0);
	}
	maxduty = PWM_COUNT_TOP;
}

HardwarePWM::~HardwarePWM()
{
	for(unsigned i = 0; i < channel_count; ++i) {
		auto slice_num = pwm_gpio_to_slice_num(channels[i]);
		pwm_set_enabled(slice_num, false);
	}
}

uint32_t HardwarePWM::getDutyChan(uint8_t chan)
{
	if(chan >= channel_count) {
		return 0;
	}
	auto pin = channels[chan];
	auto slice_num = pwm_gpio_to_slice_num(pin);
	return pwm_hw->slice[slice_num].top;
}

bool HardwarePWM::setDutyChan(uint8_t chan, uint32_t duty, bool update)
{
	if(chan >= channel_count) {
		return false;
	}
	auto pin = channels[chan];
	duty = std::min(duty, maxduty);
	pwm_set_gpio_level(pin, duty);
	return true;
}

uint32_t HardwarePWM::getPeriod()
{
	// TODO
	return 0;
}

void HardwarePWM::setPeriod(uint32_t period)
{
	// TODO
}

void HardwarePWM::update()
{
	// TODO
}

uint32_t HardwarePWM::getFrequency(uint8_t pin)
{
	// TODO
	return 0;
}
