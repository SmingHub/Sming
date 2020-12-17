/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __PWM_H__
#define __PWM_H__

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

/*pwm.h: function and macro definition of PWM API , driver level */
/*user_light.h: user interface for light API, user level*/
/*user_light_adj: API for color changing and lighting effects, user level*/

/*NOTE!!  : DO NOT CHANGE THIS FILE*/

/*SUPPORT UP TO 8 PWM CHANNEL*/
#define PWM_CHANNEL_NUM_MAX 8

//struct pwm_param {
//	uint32_t period;
//	uint32_t freq;
//	uint32_t duty[PWM_CHANNEL_NUM_MAX]; //PWM_CHANNEL<=8
//};
//
///* pwm_init should be called only once, for now  */
//void pwm_init(uint32_t period, uint32_t* duty, uint32_t pwm_channel_num, uint32_t (*pin_info_list)[3]);
//void pwm_start(void);
//
//void pwm_set_duty(uint32_t duty, uint8_t channel);
//uint32_t pwm_get_duty(uint8_t channel);
//void pwm_set_period(uint32_t period);
//uint32_t pwm_get_period(void);
//
//uint32_t get_pwm_version(void);
//void set_pwm_debug_en(uint8_t print_en);

#if defined(__cplusplus)
}
#endif

#endif
