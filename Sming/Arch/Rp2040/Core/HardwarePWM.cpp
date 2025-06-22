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
#include <hardware/clocks.h>
#include <muldiv.h>
#include <algorithm>
#include <debug_progmem.h>

/*
 * Note on use of divisor
 * ----------------------
 *
 * Divisor is 8:4 fractional value, so 1 <= int <= 255, 0 <= frac <= 15
 * Simplest way to use full range is to factor calculations by 16.
 *
 * Using default CSR_PH_CORRECT=0:
 *
 * 		F_PWM = 16 * F_SYS / (TOP + 1) / DIV
 *
 */

#define PWM_FREQ_DEFAULT 1000

HardwarePWM::HardwarePWM(const uint8_t* pins, uint8_t no_of_pins, bool usePhaseShift ) : channel_count(no_of_pins)
{
	assert(no_of_pins > 0 && no_of_pins <= PWM_CHANNEL_NUM_MAX);
	no_of_pins = std::min(uint8_t(PWM_CHANNEL_NUM_MAX), no_of_pins);
	std::copy_n(pins, no_of_pins, channels);
	setPeriod(1e6 / PWM_FREQ_DEFAULT);

	for(unsigned i = 0; i < no_of_pins; ++i) {
		auto pin = channels[i];
		gpio_set_function(pin, GPIO_FUNC_PWM);
		gpio_set_dir(pin, GPIO_OUT);
	}
}

HardwarePWM::~HardwarePWM()
{
	for(unsigned i = 0; i < channel_count; ++i) {
		auto slice_num = pwm_gpio_to_slice_num(channels[i]);
		pwm_set_enabled(slice_num, false);
	}
}

uint32_t HardwarePWM::getDutyChan(uint8_t chan) const
{
	if(chan >= channel_count) {
		return 0;
	}
	auto pin = channels[chan];
	auto slice_num = pwm_gpio_to_slice_num(pin);
	auto value = pwm_hw->slice[slice_num].cc;
	value >>= pwm_gpio_to_channel(pin) ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB;
	return value & 0xffff;
}

bool HardwarePWM::setDutyChan(uint8_t chan, uint32_t duty, bool)
{
	if(chan >= channel_count) {
		return false;
	}
	auto pin = channels[chan];
	duty = std::min(duty, maxduty);
	pwm_set_gpio_level(pin, duty);
	return true;
}

uint32_t HardwarePWM::getPeriod() const
{
	// All channels configured with same clock
	auto slice_num = pwm_gpio_to_slice_num(channels[0]);
	uint32_t top = pwm_hw->slice[slice_num].top;
	uint32_t div = pwm_hw->slice[slice_num].div;
	return muldiv(62500ULL, (top + 1) * div, clock_get_hz(clk_sys));
}

void HardwarePWM::setPeriod(uint32_t period)
{
	const uint32_t topMax{0xffff};
	const uint32_t divMin{0x10};  // 1.0
	const uint32_t divMax{0xfff}; // INT + FRAC
	auto sysFreq = clock_get_hz(clk_sys);
	// Calculate divisor assuming maximum value for TOP: ensure value is rounded UP
	uint32_t div = ((uint64_t(period) * sysFreq / 62500) + topMax) / (topMax + 1);
	uint32_t top;
	if(div > divMax) {
		// Period too big, set to maximum
		top = topMax;
		div = divMax;
	} else {
		if(div < divMin) {
			// Period is very small, set div to minimum
			div = divMin;
		}
		top = (uint64_t(period) * sysFreq / 62500 / div) - 1;
	}

	debug_d("[PWM] %s(%u): div %u, top %u", __FUNCTION__, period, div, top);

	pwm_config cfg = pwm_get_default_config();
	cfg.div = div;
	cfg.top = top;

	for(unsigned i = 0; i < channel_count; ++i) {
		auto pin = channels[i];
		auto slice_num = pwm_gpio_to_slice_num(pin);
		pwm_init(slice_num, &cfg, true);
	}

	maxduty = top;
}

void HardwarePWM::update()
{
	// Not implemented
}

uint32_t HardwarePWM::getFrequency(uint8_t pin) const
{
	auto slice_num = pwm_gpio_to_slice_num(pin);
	auto top = pwm_hw->slice[slice_num].top;
	auto div = pwm_hw->slice[slice_num].div;
	return 16UL * clock_get_hz(clk_sys) / (div * (top + 1));
}
