/* 
 * File:   HW_PWM.cpp
 * Author: https://github.com/hrsavla
 *
 * This HW_PWM library enables Sming framework user to use ESP SDK PWM API 
 * Period of PWM is fixed to 1000ms / Frequency = 1khz  
 * Duty at 100% = 22222. Duty at 0% = 0
 * You can use function setPeriod() to change frequency/period. 
 * Calculate the Duty as per the formulae give in ESP8266 SDK
 * Duty = (Period *1000)/45
 * 
 * PWM can be generated on upto  8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 */
#include "../SmingCore/HW_PWM.h"
#include "../SmingCore/Clock.h"
#include "../Wiring/WiringFrameworkIncludes.h"

HWPWM::HWPWM(uint8_t* pins, int no_of_pins) {
    if (no_of_pins > 0) {
        for (uint8_t i = 0; i < no_of_pins; i++) {
            io_info[i][0] = EspDigitalPins[pins[i]].mux;
            io_info[i][1] = EspDigitalPins[pins[i]].gpioFunc;
            io_info[i][2] = EspDigitalPins[pins[i]].id;
            pwm_duty_init[i] = 0; // Start with zero output
            channel[i] = pins[i];
        }
        pwm_init(1000,pwm_duty_init,no_of_pins,io_info);
        pwm_start(); // Lets not start pwm just after init
    }
}

HWPWM::~HWPWM() {
    // There are not functions in SDK to stop PWM output. Let's hope they put 
    // it in future.
}

/* Function Name: getChannel
 * Description: This function is used to get channel number for given pin
 * Parameters: pin -Esp8266 pin number
 */
uint8_t HWPWM::getChannel(uint8_t pin)
{
    for(uint8_t i=0;i<PWM_CHANNEL_NUM_MAX; i++)
    {
        if (channel[i] == pin)
        {
          //  debugf("%d is %d", pin, i);
            return i;
        }
    }
    return 9; // return invalid number
}

/* Function Name: analogWrite
 * Description: This function is used set pwm duty cycle for given pin
 * Parameters: pin -Esp8266 pin number
 * 			   duty - duty cycle value (valid values are 0-22222)
 * Default frequency is 1khz but can be varied by various function
 */
void HWPWM::analogWrite(uint8_t pin, uint32 duty)
{

	uint8_t i = getChannel(pin);
	if (duty <= 22222)
    {
		pwm_set_duty( duty, i);
		pwm_start();
		delay(25); // delay is needed otherwise PWM is not set.
    }
	else
	{
		debugf("Duty cycle value too high. Should be less than 22222.");
	}
}

/* Function Name: getDuty
 * Description: This function is used to get Duty cycle number for given pin
 * Parameters: pin -Esp8266 pin number
 */
uint32 HWPWM::getDuty(uint8 pin)
{
   return pwm_get_duty(pin);
}

/* Function Name: setDuty
 * Description: This function is used to get channel number for given pin
 * Parameters: pin -Esp8266 pin number
 * 			   duty - duty cycle value (Valid number are 0-22222)
 */
void HWPWM::setDuty(uint32 duty, uint8 pin)
{
    uint8_t channel =  getChannel(pin);
    if (duty < 22222)
    {
    pwm_set_duty(duty, channel);
    pwm_start();
    delay(25);
    }
}

/* Function Name: getPeriod
 * Description: This function is used to get Period of PWM.
 * 				Period / frequency will remain same for all pins.
 *
 */
uint32 HWPWM::getPeriod()
{
    return pwm_get_period();
}

/* Function Name: setPeriod
 * Description: This function is used to set Period of PWM.
 * 				Period / frequency will remain same for all pins.
 */
void HWPWM::setPeriod(uint32 period)
{
    pwm_set_period(period);
    pwm_start();
}
