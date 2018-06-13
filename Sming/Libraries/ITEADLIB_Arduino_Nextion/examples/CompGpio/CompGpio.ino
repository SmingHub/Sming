/**
 * @example CompGpio.ino
 *
 * @par How to Use
 * This example shows that In nextion screen displays the current 
 * IO mouth level change, to show how to use the API.
 *
 * @author  huangxiaoming (email:<xiaoming.huang@itead.cc>)
 * @date    2016/12/8
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include "Nextion.h"

NexGpio gpio;
NexNumber n0 = NexNumber(0,1,"n0");
NexNumber n1 = NexNumber(0,3,"n1");

#define   GPIO_PUSH_PORT          1
#define   GPIO_PWM_PORT           2
#define   GPIO_PUSH_OUTPUT_MODE   2
#define   GPIO_PWM_OUT_MODE       3
#define   CONTROLS_ID             0           //when the modeel is 1 to be valid

uint32_t pwm_value = 0;

void setup() 
{  
    nexSerial.begin(115200);
    gpio.pin_mode(GPIO_PUSH_PORT,GPIO_PUSH_OUTPUT_MODE,CONTROLS_ID);
    gpio.pin_mode(GPIO_PWM_PORT,GPIO_PWM_OUT_MODE,CONTROLS_ID);
}

void loop() 
{
    if(gpio.digital_read(1) == 0)
    {
        gpio.digital_write(GPIO_PUSH_PORT,HIGH);
        n0.setValue(1);
    }
    else
    {
        gpio.digital_write(GPIO_PUSH_PORT,LOW);
        n0.setValue(0);
    }

    
    gpio.analog_write(GPIO_PWM_PORT,pwm_value);
    n1.setValue(pwm_value);
    if(pwm_value == 100)
    {
        pwm_value = 0;
    }
    else
    {
        pwm_value += 20;
    }

    delay(1000);
}
