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
#include "hal/ledc_types.h"
#include <HardwarePWM.h>

namespace{
	ledc_mode_t pinToGroup(uint8_t pin);
	ledc_channel_t pinToChannel(uint8_t pin);
	ledc_timer_t pinToTimer(uint8_t pin);
	uint32_t periodToFrequency(uint32_t period);
	uint32_t frequencyToPeriod(uint32_t freq);
	uint32_t maxDuty(ledc_timer_bit_t bits);
}

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
	debug_d("starting HardwarePWM init");
	periph_module_enable(PERIPH_LEDC_MODULE);
	if((no_of_pins == 0) || (no_of_pins > SOC_LEDC_CHANNEL_NUM))
	{
		// is there a return code for this?
		return ;
	}
	uint32_t io_info[SOC_LEDC_CHANNEL_NUM][3];	// pin information
	uint32_t pwm_duty_init[SOC_LEDC_CHANNEL_NUM]; // pwm duty
	for(uint8_t i = 0; i < no_of_pins; i++) {
		
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

		debug_d("ledc_timer.\n\tspeed_mode: %i\n\ttimer_num: %i\n\tduty_resolution: %i\n\tfreq: %i\n\tclk_cfg: %i\n\n",
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
		debug_d("ledc_channel\n\tspeed_mode: %i\n\tchannel: %i\n\ttimer_sel %i\n\tinr_type: %i\n\tgpio_num: "
				"%i\n\tduty: %i\n\thpoint: %i\n\n",
				pinToGroup(i) , pinToChannel(i), pinToTimer(i), LEDC_INTR_DISABLE, pins[i], 0, 0);
		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
		ledc_bind_channel_timer(pinToGroup(i), pinToChannel(i), pinToTimer(i));

		maxduty=maxDuty(DEFAULT_RESOLUTION);
		const int initial_period=DEFAULT_PERIOD;
	}
}

HardwarePWM::~HardwarePWM()
{
	for(uint8_t i = 0; i < no_of_pins; i++) {
		//stop pwm for all pins and set idle level to 0.
		
	ledc_stop(pinToGroup(pins[i]), pinToChannel(pins[i]), uint32_t 0); 
	}
}

/* Function Name: getChannel
 * Description: This function is used to get channel number for given pin
 * Parameters: pin - Esp8266 pin number
 */
uint8_t HardwarePWM::getChannel(uint8_t pin)
{
	for(uint8_t i = 0; i < channel_count; i++) {
		if(channels[i] == pin) {
			//debug_d("getChannel %d is %d", pin, i);
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
		debug_d("Duty cycle value too high for current period.");
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
	if(period!=0){
		return (1000000 / period);
	}
}

uint32_t HardwarePWM::frequencyToPeriod(uint32_t freq){
	if(freq!=0){
		return (1000000 / freq);
	}
}

uint32_t HardwarePWM::maxDuty(ledc_timer_bit_t bits){
	return (1<<(uint32_t)bits) - 1; 
}

uint32_t HardwarePWM::getFrequency(uint8_t pin){
	return ledc_get_freq(pinToGroup(pin), pinToTimer(pin));
}