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
 * esp32s3 SOC_LEDC_CHANNEL_NUM 		(8)
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
 * - implement the "vanilla" Sming HardwarePWM i
 nterface that will hide the underlying architecture but allow up to 16 
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

// orig #include <Clock.h>
// orig #include "ESP8266EX.h"
#include <debug_progmem.h>
//#include <soc/soc_caps.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include <HardwarePWM.h>


// orig #include <HardwarePWM.h>

// #define SOC_LEDC_TIMER_BIT_WIDE_NUM 9 
// #define SOC_LEDC_CHANNEL_NUM 6


 // default period in µs -> 5kHz


// define default resolution

/*#if static_cast<ledc_timer_bit_t>SOC_LEDC_TIMER_BIT_WIDE_NUM >= DESIRED_DEFAULT_RESOLUTION
        #define DEFAULT_RESOLUTION DESIRED_DEFAULT_RESOLUTION
#else
        #define DEFAULT_RESOLUTION static_cast<ledc_timer_bit_t>SOC_LEDC_TIMER_BIT_WIDE_NUM // if 10 bit is not available, set to max
#endif
*/
#define DEFAULT_RESOLUTION static_cast<ledc_timer_bit_t>(10)
#define DEFAULT_PERIOD 200

HardwarePWM::HardwarePWM(uint8_t* pins, uint8_t no_of_pins) : channel_count(no_of_pins)
{
	ledc_timer_config_t ledc_timer;
	ledc_channel_config_t ledc_channel;
	debugf("starting HardwarePWM init");
	periph_module_enable(PERIPH_LEDC_MODULE);
	if((no_of_pins > 0) && (no_of_pins < SOC_LEDC_CHANNEL_NUM))
	{
		uint32_t io_info[SOC_LEDC_CHANNEL_NUM][3];	// pin information
		uint32_t pwm_duty_init[SOC_LEDC_CHANNEL_NUM]; // pwm duty
		for(uint8_t i = 0; i < no_of_pins; i++) {
			/* todo: update the digitalPins map
			io_info[i][0] = EspDigitalPins[pins[i]].mux;
			io_info[i][1] = EspDigitalPins[pins[i]].gpioFunc;
			io_info[i][2] = EspDigitalPins[pins[i]].id;
			*/
			pwm_duty_init[i] = 0; // Start with zero output
			channels[i] = pins[i];

			/* 
			/  Prepare and then apply the LEDC PWM timer configuration
			/  this may cofigure the same timer more than once (in fact up to 8 times)
			/  which should not be an issue, though, since the values should be the same for all timers
			*/
			ledc_timer.speed_mode      = pinToGroup(i);  // the two groups (if available) are operating in different speed modes, hence speed mode is an alias for group or vice versa
			ledc_timer.timer_num       = pinToTimer(i);
			ledc_timer.duty_resolution = LEDC_TIMER_10_BIT; // todo: make configurable later
			ledc_timer.freq_hz         = periodToFrequency(DEFAULT_PERIOD); // todo: make configurable later
			ledc_timer.clk_cfg         = LEDC_AUTO_CLK;

			debugf("ledc_timer.\n\tspeed_mode: %i\n\ttimer_num: %i\n\tduty_resolution: %i\n\tfreq: %i\n\tclk_cfg: %i\n\n",
				   (uint32_t)ledc_timer.speed_mode, (uint32_t)ledc_timer.timer_num,
				   (uint32_t)ledc_timer.duty_resolution, (uint32_t)ledc_timer.freq_hz, (uint32_t)ledc_timer.clk_cfg);
			ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

			/*
			/   Prepare and then apply the LEDC PWM channel configuration
			*/
			ledc_channel.speed_mode     = pinToGroup(i);
			ledc_channel.channel        = pinToChannel(i);
			ledc_channel.timer_sel      = pinToTimer(i);
			ledc_channel.intr_type      = LEDC_INTR_DISABLE;
			ledc_channel.gpio_num       = pins[i];
			ledc_channel.duty           = 0; // Set duty to 0%
			ledc_channel.hpoint         = 0;
			debugf("ledc_channel\n\tspeed_mode: %i\n\tchannel: %i\n\ttimer_sel %i\n\tinr_type: %i\n\tgpio_num: "
				   "%i\n\tduty: %i\n\thpoint: %i\n\n",
				   pinToGroup(i) , pinToChannel(i), pinToTimer(i), LEDC_INTR_DISABLE, pins[i], 0, 0);
			ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
			ledc_bind_channel_timer(pinToGroup(i), pinToChannel(i), pinToTimer(i));
		}
		maxduty=maxDuty(DEFAULT_RESOLUTION);
		const int initial_period=DEFAULT_PERIOD;
	}
}

HardwarePWM::~HardwarePWM()
{
	// There is no function in the SDK to stop PWM output, yet.
}

/* Function Name: getChannel
 * Description: This function is used to get channel number for given pin
 * Parameters: pin - Esp8266 pin number
 */
uint8_t HardwarePWM::getChannel(uint8_t pin)
{
	for(uint8_t i = 0; i < channel_count; i++) {
		if(channels[i] == pin) {
			//debugf("getChannel %d is %d", pin, i);
			return i;
		}
	}
	return -1;
}

/* Function Name: getDutyChan
 * Description: This function is used to get the duty cycle number for a given channel
 * Parameters: chan -Esp8266 channel number
 */
uint32_t HardwarePWM::getDutyChan(uint8_t chan)
{
	if(chan == PWM_BAD_CHANNEL) {
		return 0;
	} else {
		return ledc_get_duty(pinToGroup(chan),pinToChannel(chan));		
	}
		// esp32 defines the frequency / period per timer,  
}

/* Function Name: setDutyChan
 * Description: This function is used to set the pwm duty cycle for a given channel. If parameter 'update' is false
 *              then you have to call update() later to update duties.
 * Parameters: chan - channel number
 *             duty - duty cycle value
 *             update - update PWM output
 */
bool HardwarePWM::setDutyChan(uint8_t chan, uint32_t duty, bool update)
{
	if(chan == PWM_BAD_CHANNEL) {
		return false;
	} else if(duty <= maxduty) {
		ESP_ERROR_CHECK(ledc_set_duty(pinToGroup(chan), pinToChannel(chan), duty));
		if(update) {
			ESP_ERROR_CHECK(ledc_update_duty(pinToGroup(chan), pinToChannel(chan)));
			//update();
		}
		return true;
	} else {
		debugf("Duty cycle value too high for current period.");
		return false;
	}
}

/* Function Name: getPeriod
 * Description: This function is used to get Period of PWM.
 *				Period / frequency will remain same for all pins.
 *
 */
uint32_t HardwarePWM::getPeriod()
{
	// sming does not know how to handle different frequencies for channels, this will require an extended interface
	// for now, just report the period for group 0 channel 0 
	return frequencyToPeriod(ledc_get_freq(static_cast<ledc_mode_t>(0),static_cast<ledc_timer_t>(0)));
}

/* Function Name: setPeriod
 * Description: This function is used to set Period of PWM.
 *				Period / frequency will remain same for all pins.
 */
void HardwarePWM::setPeriod(uint32_t period)
{
	// setting the frequency globally, will add per timer functions later
	// also, this can be done smarter 
	for(uint8_t i = 0; i < channel_count; i++) {
		ESP_ERROR_CHECK(ledc_set_freq(pinToGroup(i), pinToTimer(i), periodToFrequency(period)));
	}
	//sledc_update_duty();
	update();
}

/* Function Name: update
 * Description: This function is used to actually update the PWM.
 */
void HardwarePWM::update()
{
	//ledc_update_duty();	
}

ledc_channel_t HardwarePWM::pinToChannel(uint8_t pin){
	return (ledc_channel_t)(pin % 8);
}

ledc_mode_t HardwarePWM::pinToGroup(uint8_t pin){
	return (ledc_mode_t) (pin / 8);
}

ledc_timer_t HardwarePWM::pinToTimer(uint8_t pin){
	return (ledc_timer_t) ((pin /2) % 4);
}

uint32_t HardwarePWM::periodToFrequency(uint32_t period){
	return (1000000 / period);
}

uint32_t HardwarePWM::frequencyToPeriod(uint32_t freq){
	return (1000000 / freq);
}

uint32_t HardwarePWM::maxDuty(ledc_timer_bit_t bits){
	return (1<<(uint32_t)bits) - 1; 
}

uint32_t HardwarePWM::getFrequency(uint8_t pin){
	return ledc_get_freq(pinToGroup(pin), pinToTimer(pin));
}