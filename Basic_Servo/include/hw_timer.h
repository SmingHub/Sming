/*
 * hw_timer.h
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */

#ifndef INCLUDE_HW_TIMER_H_
#define INCLUDE_HW_TIMER_H_

/******************************************************************************
* Copyright 2013-2014 Espressif Systems (Wuxi)
*
* FileName: hw_timer.c
*
* Description: hw_timer driver
*
* Modification history:
*     2014/5/1, v1.0 create this file.
*******************************************************************************/
//#include "ets_sys.h"
//#include "os_type.h"
#include "osapi.h"


typedef enum {
	NO_RELOAD = 0,
	AUTO_RELOAD = 1,
} RELOAD_TYPE;


typedef enum {
    FRC1_SOURCE = 0,
    NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

/******************************************************************************
* FunctionName : hw_timer_arm
* Description  : set a trigger timer delay for this timer.
* Parameters   : uint32 val（时间单位：us）
in autoload mode
                        50 ~ 0x7fffff;  for FRC1 source.
                        100 ~ 0x7fffff;  for NMI source.
in non autoload mode:
                        10 ~ 0x7fffff;
* Returns      : NONE
*******************************************************************************/
void  hw_timer_arm(u32 val);


/******************************************************************************
* FunctionName : hw_timer_set_func
* Description  : set the func, when trigger timer is up.
* Parameters   : void (* user_hw_timer_cb_set)(void):
                        timer callback function,
* Returns      : NONE
*******************************************************************************/
void  hw_timer_set_func(void (* user_hw_timer_cb_set)(void));

/******************************************************************************
* FunctionName : hw_timer_init
* Description  : initilize the hardware isr timer
* Parameters   :
FRC1_TIMER_SOURCE_TYPE source_type:
                        FRC1_SOURCE,    timer use frc1 isr as isr source.
                        NMI_SOURCE,     timer use nmi isr as isr source.
u8 req:
                        0,  not autoload,
                        1,  autoload mode,
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR hw_timer_init(FRC1_TIMER_SOURCE_TYPE source_type, u8 req);


/*
NOTE:
1 if use nmi source, for autoload timer , the timer setting val can't be less than 100.
2 if use nmi source, this timer has highest priority, can interrupt other isr.
3 if use frc1 source, this timer can't interrupt other isr.

*/




#endif /* INCLUDE_HW_TIMER_H_ */
