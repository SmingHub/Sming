/**
 * @file NexGpio.h
 *
 * The definition of class NexGpio. 
 *
 * @author Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date 2015/8/13
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _NEXGPIO_H
#define _NEXGPIO_H
     
#include "NexTouch.h"
#include "NexHardware.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexGpio component.
 */

class NexGpio
{
public:
    /**
     * Set gpio mode
     *
     * @param port - the gpio port number
     * @param mode - set gpio port mode(0--Pull on the input
     *                                  1--the control input binding
     *                                  2--Push-pull output
     *                                  3--pwm output
     *                                  4--open mode leakage)
     * @param control_id - nextion controls id ,when the modeel is 1 to be valid
     * @return true if success, false for failure
     */
    
    bool pin_mode(uint32_t port,uint32_t mode,uint32_t control_id);
    
    /**
     * write a  HIGH or a LOW value to a digital pin
     *
     * @param port - the gpio port number
     * @param value - HIGH or LOW
     * @return true if success, false for failure
     */
    
    bool digital_write(uint32_t port,uint32_t value);
    
    /**
     * read a HIGH or a LOW value to a digital pin
     *
     * @param port - the gpio port number
     * @return the value from a specified digital pin, either high or low 
     */
    
    uint32_t digital_read(uint32_t port);
    
    /**
     * writes an analog value (PWM wave) to a pin
     *
     * @param port - the gpio port number
     * @param value - the duty cycle: between 0 (always off) and 100 (always on).
     * @return true if success, false for failure
     */
    
    bool analog_write(uint32_t port,uint32_t value);
    
    /**
     * writes pwm output frequency
     *
     * @param value - the frequency: between 1 and 65535
     * @return true if success, false for failure
     */
    
    bool set_pwmfreq(uint32_t value);
    
    /**
     * read pwm output frequency
     *
     * @param number - the frequency
     * @return true if success, false for failure
     */
    
    uint32_t get_pwmfreq(uint32_t *number);
     
};
    
/**
 * @}
 */    
    
#endif /* #ifndef __NEXGPIO_H__ */