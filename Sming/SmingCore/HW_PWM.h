/* 
 * File:   HW_PWM.h
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

#ifndef HW_PWM_H
#define	HW_PWM_H


#include "../SmingCore/ESP8266EX.h"
#include "../Wiring/WiringFrameworkDependencies.h"

#include "../system/include/espinc/pwm.h" // SDK PWM API file
//#include "../ESP8266EX.h"
class HWPWM
{
public:
    HWPWM(uint8_t *pins, int no_of_pins);
    virtual ~HWPWM();
    void analogWrite(uint8_t pin, uint32 duty);
    void setDuty(uint32 duty, uint8 pin);
    uint32 getDuty(uint8 pin);
    void setPeriod(uint32 period);
    uint32 getPeriod(void);
    uint8_t channel[PWM_CHANNEL_NUM_MAX];
    uint8_t getChannel(uint8_t pin);
protected:
    
private:
    uint32 io_info[PWM_CHANNEL_NUM_MAX][3]; // pin information
    uint32 pwm_duty_init[PWM_CHANNEL_NUM_MAX]; // pwm duty
   
};


#endif	/* HW_PWM_H */

