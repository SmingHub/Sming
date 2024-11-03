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
#include <algorithm>
#include <debug_progmem.h>

/*
Divisor is 8:4 fractional value, so 1 <= int <= 255, 0 <= frac <= 15
Thus (2, 1) represents 2 1/16, 2.5 is (1, 8), etc.

We can also say (CLK_SYS * 16 / (CLKDIV * 16))
DIV = (DIV_INT + DIV_FRAC / 16)

Setting CSR_PH_CORRECT=0:

F_PWM = F_SYS / ((TOP + 1) * DIV)

TOP  = (F_SYS / F_PWM / DIV) - 1

period (us) = 1e6 / F_PWM
period = 1e6 / (F_SYS / ((TOP + 1) * DIV))
period = 1e6 * ((TOP + 1) * DIV) / F_SYS


1e6 * ((TOP + 1) * DIV) = 1e6 / F_SYS


*/
#define PWM_FREQ_DEFAULT 1000

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t noOfPins) : channel_count(noOfPins)
{
	assert(noOfPins > 0 && noOfPins <= PWM_CHANNEL_NUM_MAX);
	noOfPins = std::min(uint8_t(PWM_CHANNEL_NUM_MAX), noOfPins);
	std::copy_n(pins, noOfPins, channels);
	setPeriod(1e6 / PWM_FREQ_DEFAULT);

	for(unsigned i = 0; i < noOfPins; ++i) {
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

uint32_t HardwarePWM::getDutyChan(uint8_t chan)
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

uint32_t HardwarePWM::getPeriod()
{
	// All channels configured with same clock
	auto slice_num = pwm_gpio_to_slice_num(channels[0]);
	uint32_t top = pwm_hw->slice[slice_num].top;
	uint32_t div = pwm_hw->slice[slice_num].div >> PWM_CH0_DIV_INT_LSB;
	uint32_t t = (top + 1) * div;
	uint32_t clk_sys_khz = clock_get_hz(clk_sys);
	return 1'000'000ULL * t / clk_sys_khz;
}

void HardwarePWM::setPeriod(uint32_t period)
{
	/*
		TOP = (period * F_SYS / DIV / 1e6) - 1
		DIV = period * F_SYS / 1e6 / (TOP + 1)

		Range of DIV is 1 - 255
		Range of TOP is 0 - 65535

	Calculate value for DIV to maximise value TOP.

	DIV = period * F_SYS / 1e6 / (65535 + 1)

	DIV must be > 0
		=> period * F_SYS / 1e6 / (65535 + 1) > 0
		=> period * F_SYS / 1e6 >= (65535 + 1)
		=> period * F_SYS >= 1e6 * (65535 + 1)
		=> period >= 1e6 * (65535 + 1) / F_SYS

	At 125MHz period <= 524.288us
	*/

	// TODO: Use fractional part of divider

	const uint32_t topMax{0xffff};
	const uint32_t divMax{0xff};
	auto sysFreq = clock_get_hz(clk_sys);
	uint32_t div = uint64_t(period) * sysFreq / 1e6 / (topMax + 1);
	uint32_t top;
	if(div > divMax) {
		// Period too big, set to maximum
		top = topMax;
		div = divMax;
	} else {
		if(div == 0) {
			// Period is very small, set div to minimum
			div = 1;
		}
		top = (uint64_t(period) * sysFreq / div / 1e6) - 1;
		if(top > topMax) {
			debug_d("[PWM] TOP too big %u, div %u", __FUNCTION__, top, div);
			++div;
			top = (uint64_t(period) * sysFreq / div / 1e6) - 1;
		}
	}

	debug_d("[PWM] %s(%u): div %u, top %u", __FUNCTION__, period, div, top);

	pwm_config cfg = pwm_get_default_config();
	pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_FREE_RUNNING);
	pwm_config_set_clkdiv_int_frac(&cfg, div, 0);
	pwm_config_set_wrap(&cfg, top);

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

uint32_t HardwarePWM::getFrequency(uint8_t pin)
{
	auto slice_num = pwm_gpio_to_slice_num(pin);
	auto top = pwm_hw->slice[slice_num].top;
	auto div = pwm_hw->slice[slice_num].div >> PWM_CH0_DIV_INT_LSB;
	return clock_get_hz(clk_sys) / (div * (top + 1));
}
