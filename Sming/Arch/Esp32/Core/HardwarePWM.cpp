/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwarePWM.cpp
 *
 * Original Author: https://github.com/hrsavla
 * Esp32 version:   https://github.com/pljakobs
 *
 * This HardwarePWM library enables Sming framework uses to use the ESP32 ledc PWM api
 * 
 * the ESP32 PWM Hardware is much more powerful than the ESP8266, allowing wider PWM timers (up to 20 bit)
 * as well as much higher PWM frequencies (up to 40MHz for a 1 Bit wide PWM)
 * 
 * Overview:
 * +------------------------------------------------------------------------------------------------+
 * | LED_PWM                                                                                        |
 * |  +-------------------------------------------+   +-------------------------------------------+ |
 * |  | High_Speed_Channels¹                      |   | Low_Speed_Channels                        | |
 * |  |                   +-----+     +--------+  |   |                   +-----+     +--------+  | |
 * |  |                   |     | --> | h_ch 0 |  |   |                   |     | --> | l_ch 0 |  | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  | | h_timer 0 | --> |     |                 |   | | l_timer 0 | --> |     |                 | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  |                   |     | --> | h_ch 1 |  |   |                   |     | --> | l_ch 1 |  | |
 * |  |                   |     |     +--------+  |   |                   |     |     +--------+  | |
 * |  |                   |     |                 |   |                   |     |                 | |
 * |  |                   |     |     +--------+  |   |                   |     |     +--------+  | |
 * |  |                   |     | --> | h_ch 2 |  |   |                   |     | --> | l_ch 2 |  | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  | | h_timer 1 | --> |     |                 |   | | l_timer 1 | --> |     |                 | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  |                   |     | --> | h_ch 3 |  |   |                   |     | --> | l_ch 3 |  | |
 * |  |                   |     |     +--------+  |   |                   |     |     +--------+  | |
 * |  |                   | MUX |                 |   |                   | MUX |                 | |
 * |  |                   |     |     +--------+  |   |                   |     |     +--------+  | |
 * |  |                   |     | --> | h_ch 4 |  |   |                   |     | --> | l_ch 4 |  | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  | | h_timer 2 | --> |     |                 |   | | l_timer 2 | --> |     |                 | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  |                   |     | --> | h_ch 5 |  |   |                   |     | --> | l_ch 5 |  | |
 * |  |                   |     |     +--------+  |   |                   |     |     +--------+  | |
 * |  |                   |     |                 |   |                   |     |                 | |
 * |  |                   |     |     +--------+  |   |                   |     |     +--------+  | |
 * |  |                   |     | --> | h_ch 6 |  |   |                   |     | --> | l_ch 6²|  | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  | | h_timer 3 | --> |     |                 |   | | l_timer 3 | --> |     |                 | |
 * |  | +-----------+     |     |     +--------+  |   | +-----------+     |     |     +--------+  | |
 * |  |                   |     | --> | h_ch 7 |  |   |                   |     | --> | l_ch 7²|  | |
 * |  |                   |     |     +--------+  |   |                   |     |     +--------+  | |
 * |  |                   +-----+                 |   |                   +-----+                 | |
 * |  +-------------------------------------------+   +-------------------------------------------+ |
 * +------------------------------------------------------------------------------------------------+
 * ¹ High speed channels are only available when SOC_LEDC_SUPPORT_HS_MODE is defined as 1
 * ² The ESP32C3 does only support six channels, so 6 and 7 are not available on that SoC
 * 
 * The nomenclature of timers in the high speed / low speed blocks is a bit misleading as the idf api 
 * speaks of "speed mode", which, to me, implies that this would be a mode configurable in a specific timer
 * while in reality, it does select a block of timers. I am considering re-naming that to "speed mode block"
 * in my interface impmenentation.
 * 
 * As an example, I would use
 * setTimerFrequency(speedModeBlock, timer, frequency);
 * 
 * ToDo: see, how this can be implemented to provide maximum overlap with the RP2040 pwm hardware so code does 
 * not become overly SoC specific.
 * 
 * Maximum Timer width for PWM:
 * ============================
 * esp32   SOC_LEDC_TIMER_BIT_WIDE_NUM  (20)
 * esp32c3 SOC_LEDC_TIMER_BIT_WIDE_NUM  (14)
 * esp32s2 SOC_LEDC_TIMER_BIT_WIDE_NUM  (14)
 * esp32s3 SOC_LEDC_TIMER_BIT_WIDE_NUM  (14)
 * 
 * Number of Channels:
 * ===================
 * esp32   SOC_LEDC_CHANNEL_NUM         (8)
 * esp32c3 SOC_LEDC_CHANNEL_NUM         (6)
 * esp32s2 SOC_LEDC_CHANNEL_NUM         (8)
 * esp32s3 SOC_LEDC_CHANNEL_NUM 		 8
 *
 * Some SoSs support a mode called HIGHSPEED_MODE which is essentially another full block of PWM hardware 
 * that adds SOC_LEDC_CHANNEL_NUM channels. 
 * Those Architectures have SOC_LEDC_SUPPORT_HS_MODE defined as 1.
 * In esp-idf-4.3 that's currently only the esp32 SOC 
 * 
 * Supports highspeed mode:
 * ========================
 * esp32 SOC_LEDC_SUPPORT_HS_MODE	(1)
 *
 * ToDo: implement awareness of hs mode availablility
 * ==================================================
 * currently, the code just uses a % 8 operation on the pin index to calculate whether to assign a pin to either
 * high speed or low speed pwm blocks. This doesn't make a whole lot of sense since it makes it impossible
 * for Sming devs to actually use the functionality behind it. 
 * Also, it currently does not reflect the fact that different SOCs have a different number of channels per block
 * (specifically, the esp32c3 only has six channels and no highspeed mode). 
 * I will continue in two ways: 
 * - implement the "vanilla" Sming HardwarePWM interface that will hide the underlying architecture but allow up to 16 
 *   channels on an ESP32 
 * - implement overloads for the relevant functions that allow selecting hs mode where applicable. 
 * 
 * ToDo: implement PWM bit width control
 * =====================================
 * the current HardwarePWM implementation does not care about the PWM timer bit width. To leverage the functionality 
 * of the ESP32 hardware, it's necessary to make this configurable. As the width is per timer and all the Sming defined
 * functions are basically per pin, this needs to be an extension of the overal model, exposing at least timers. 
 * This, too, will require a compatible "basic" interface and an advanced interface that allows assiging pins (channels) 
 * to timers and the configuration of the timers themselves. 
 * The esp_idf does not provide a way to read the bit width configured for a channel, but I think it'll be useful to be able
 * to read back this value, not least to find the value for getMaxDuty() for a channel. It will either have to be stored in the
 * module or maybe read from the hardware directly (LEDC_[HL]STIMERx_CONF_REG & 0x1f)
 * 
 * ToDo: implement an abstraction layer
 * ====================================
 * as it stands now, this impelmentation does not provide a function to synchronize all the PWM channels (HardwarePWM::update())
 * It might be a good idea to provide an intermediary abstraction that handles all the low level PWM functions (such as flexible
 * timer to channel assignments, hs/ls mode awareness, pwm bit width etc) and implements the update() function on that level.
 * 
 * hardware technical reference: 
 * =============================
 * https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#ledpwm
 * 
 * Overview of the whole ledc-system here: 
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
 * 
 * the ledc interface also exposes some advanced functions such as fades that are then done in hardware. 
 * ToDo: implement a Sming interface for fades
 * 
 */

#include <HardwarePWM.h>
#include <debug_progmem.h>
#include <driver/periph_ctrl.h>
#include <driver/ledc.h>
#include <esp_err.h>
#include <hal/ledc_types.h>

#define DEFAULT_RESOLUTION ledc_timer_bit_t(10)
#define DEFAULT_PERIOD 200
namespace
{
ledc_channel_t pinToChannel(uint8_t pin)
{
	return ledc_channel_t(pin % 8);
}

ledc_mode_t pinToGroup(uint8_t pin)
{
	return ledc_mode_t(pin / 8);
}

ledc_timer_t pinToTimer(uint8_t pin)
{
	return ledc_timer_t((pin / 2) % 4);
}

uint32_t periodToFrequency(uint32_t period)
{
	return (period == 0) ? -1 : (1000000 / period);
}

uint32_t frequencyToPeriod(uint32_t freq)
{
	return (freq == 0) ? -1 : (1000000 / freq);
}

uint32_t maxDuty(ledc_timer_bit_t bits)
{
	return (1U << bits) - 1;
}

} //namespace

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t no_of_pins) : channel_count(no_of_pins)
{
	debug_d("starting HardwarePWM init");
	periph_module_enable(PERIPH_LEDC_MODULE);
	if((no_of_pins == 0) || (no_of_pins > SOC_LEDC_CHANNEL_NUM)) {
		return;
	}

	uint32_t io_info[SOC_LEDC_CHANNEL_NUM][3];	// pin information
	uint32_t pwm_duty_init[SOC_LEDC_CHANNEL_NUM]; // pwm duty
	for(uint8_t i = 0; i < no_of_pins; i++) {
		pwm_duty_init[i] = 0; // Start with zero output
		channels[i] = pins[i];

		/* 
		 *  Prepare and then apply the LEDC PWM timer configuration.
		 *  This may cofigure the same timer more than once (in fact up to 8 times)
		 *  which should not be an issue, though, since the values should be the same for all timers
		 */
		// The two groups (if available) are operating in different speed modes, hence speed mode is an alias for group or vice versa
		ledc_timer_config_t ledc_timer{
			.speed_mode = pinToGroup(i),
			.duty_resolution = LEDC_TIMER_10_BIT, // todo: make configurable later
			.timer_num = pinToTimer(i),
			.freq_hz = periodToFrequency(DEFAULT_PERIOD), // todo: make configurable later
			.clk_cfg = LEDC_AUTO_CLK,
		};

		debug_d("ledc_timer.\r\n"
				"\tspeed_mode: %i\r\n"
				"\ttimer_num: %i\r\n"
				"\tduty_resolution: %i\r\n"
				"\tfreq: %i\n\tclk_cfg: %i\r\n\n",
				ledc_timer.speed_mode, ledc_timer.timer_num, ledc_timer.duty_resolution, ledc_timer.freq_hz,
				ledc_timer.clk_cfg);
		ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

		/*
		 * Prepare and then apply the LEDC PWM channel configuration
		 */
		ledc_channel_config_t ledc_channel{
			.gpio_num = pins[i],
			.speed_mode = pinToGroup(i),
			.channel = pinToChannel(i),
			.intr_type = LEDC_INTR_DISABLE,
			.timer_sel = pinToTimer(i),
			.duty = 0,
			.hpoint = 0,
		};
		debug_d("ledc_channel\n"
				"\tspeed_mode: %i\r\n"
				"\tchannel: %i\r\n"
				"\ttimer_sel %i\r\n"
				"\tintr_type: %i\r\n"
				"\tgpio_num: %i\r\n"
				"\tduty: %i\r\n"
				"\thpoint: %i\r\n\n",
				pinToGroup(i), pinToChannel(i), pinToTimer(i), ledc_channel.intr_type, pins[i], 0, 0);
		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
		ledc_bind_channel_timer(pinToGroup(i), pinToChannel(i), pinToTimer(i));
	}
	maxduty = maxDuty(DEFAULT_RESOLUTION);
}

HardwarePWM::~HardwarePWM()
{
	// Stop pwm for all pins and set idle level to 0
	for(uint8_t i = 0; i < channel_count; i++) {
		ledc_stop(pinToGroup(i), pinToChannel(i), 0);
	}
}

uint8_t HardwarePWM::getChannel(uint8_t pin)
{
	for(uint8_t i = 0; i < channel_count; i++) {
		if(channels[i] == pin) {
			// debug_d("getChannel %d is %d", pin, i);
			return i;
		}
	}
	return -1;
}

uint32_t HardwarePWM::getDutyChan(uint8_t chan)
{
	// esp32 defines the frequency / period per timer
	return (chan == PWM_BAD_CHANNEL) ? 0 : ledc_get_duty(pinToGroup(chan), pinToChannel(chan));
}

bool HardwarePWM::setDutyChan(uint8_t chan, uint32_t duty, bool update)
{
	if(chan == PWM_BAD_CHANNEL) {
		return false;
	} else if(duty <= maxduty) {
		ESP_ERROR_CHECK(ledc_set_duty(pinToGroup(chan), pinToChannel(chan), duty));
		/*
		* ignoring the update flag in this release, ToDo: implement a synchronized update mechanism
		* if(update) {
		*	ESP_ERROR_CHECK(ledc_update_duty(pinToGroup(chan), pinToChannel(chan)));
		*	//update();
		* }
		*/
		ESP_ERROR_CHECK(ledc_update_duty(pinToGroup(chan), pinToChannel(chan)));
		return true;
	} else {
		debug_d("Duty cycle value too high for current period.");
		return false;
	}

	if(duty <= maxduty) {
		ESP_ERROR_CHECK(ledc_set_duty(pinToGroup(chan), pinToChannel(chan), duty));
		/*
		* ignoring the update flag in this release, ToDo: implement a synchronized update mechanism
		* if(update) {
		*	ESP_ERROR_CHECK(ledc_update_duty(pinToGroup(chan), pinToChannel(chan)));
		*	//update();
		* }
		*/
		ESP_ERROR_CHECK(ledc_update_duty(pinToGroup(chan), pinToChannel(chan)));
		return true;
	}

	debug_d("Duty cycle value too high for current period.");
	return false;
}

uint32_t HardwarePWM::getPeriod()
{
	// Sming does not know how to handle different frequencies for channels: this will require an extended interface.
	// For now, just report the period for group 0 channel 0.
	return frequencyToPeriod(ledc_get_freq(ledc_mode_t(0), ledc_timer_t(0)));
}

void HardwarePWM::setPeriod(uint32_t period)
{
	// Set the frequency globally, will add per timer functions later.
	// Also, this can be done smarter.
	for(uint8_t i = 0; i < channel_count; i++) {
		ESP_ERROR_CHECK(ledc_set_freq(pinToGroup(i), pinToTimer(i), periodToFrequency(period)));
	}
	// ledc_update_duty();
	update();
}

void HardwarePWM::update()
{
	// ledc_update_duty();
}

uint32_t HardwarePWM::getFrequency(uint8_t pin)
{
	return ledc_get_freq(pinToGroup(pin), pinToTimer(pin));
}

namespace{
	ledc_channel_t pinToChannel(uint8_t pin){
		return (ledc_channel_t)(pin % 8);
	}

	ledc_mode_t pinToGroup(uint8_t pin){
		return (ledc_mode_t) (pin / 8);
	}

	ledc_timer_t pinToTimer(uint8_t pin){
		return (ledc_timer_t) ((pin /2) % 4);
	}

	uint32_t periodToFrequency(uint32_t period){
		if(period == 0){
			return 0;
		}else{
			return (1000000 / period);
		}
	}

	uint32_t frequencyToPeriod(uint32_t freq){
		if(freq == 0) {
			return 0;
		} else {
			return (1000000 / freq);
		}
	}

	uint32_t maxDuty(ledc_timer_bit_t bits){
		return (1<<(uint32_t)bits) - 1; 
	}

}