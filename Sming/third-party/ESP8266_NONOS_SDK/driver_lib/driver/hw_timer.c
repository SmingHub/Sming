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

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"

#define US_TO_RTC_TIMER_TICKS(t)          \
    ((t) ?                                   \
     (((t) > 0x35A) ?                   \
      (((t)>>2) * ((APB_CLK_FREQ>>4)/250000) + ((t)&0x3) * ((APB_CLK_FREQ>>4)/1000000))  :    \
      (((t) *(APB_CLK_FREQ>>4)) / 1000000)) :    \
     0)

#define FRC1_ENABLE_TIMER  BIT7
#define FRC1_AUTO_LOAD  BIT6

//TIMER PREDIVED MODE
typedef enum {
    DIVDED_BY_1 = 0,		//timer clock
    DIVDED_BY_16 = 4,	//divided by 16
    DIVDED_BY_256 = 8,	//divided by 256
} TIMER_PREDIVED_MODE;

typedef enum {			//timer interrupt mode
    TM_LEVEL_INT = 1,	// level interrupt
    TM_EDGE_INT   = 0,	//edge interrupt
} TIMER_INT_MODE;

typedef enum {
    FRC1_SOURCE = 0,
    NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

/******************************************************************************
* FunctionName : hw_timer_arm
* Description  : set a trigger timer delay for this timer.
* Parameters   : uint32 val :
in autoload mode
                        50 ~ 0x7fffff;  for FRC1 source.
                        100 ~ 0x7fffff;  for NMI source.
in non autoload mode:
                        10 ~ 0x7fffff;
* Returns      : NONE
*******************************************************************************/
void  hw_timer_arm(u32 val)
{
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, US_TO_RTC_TIMER_TICKS(val));
}

static void (* user_hw_timer_cb)(void) = NULL;
/******************************************************************************
* FunctionName : hw_timer_set_func
* Description  : set the func, when trigger timer is up.
* Parameters   : void (* user_hw_timer_cb_set)(void):
                        timer callback function,
* Returns      : NONE
*******************************************************************************/
void  hw_timer_set_func(void (* user_hw_timer_cb_set)(void))
{
    user_hw_timer_cb = user_hw_timer_cb_set;
}

static void hw_timer_isr_cb(void *arg)
{
    if (user_hw_timer_cb != NULL) {
        (*(user_hw_timer_cb))();
    }
}

static void hw_timer_nmi_cb(void)
{
    if (user_hw_timer_cb != NULL) {
        (*(user_hw_timer_cb))();
    }
}

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
void ICACHE_FLASH_ATTR hw_timer_init(FRC1_TIMER_SOURCE_TYPE source_type, u8 req)
{
    if (req == 1) {
        RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                      FRC1_AUTO_LOAD | DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
    } else {
        RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                      DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
    }

    if (source_type == NMI_SOURCE) {
        ETS_FRC_TIMER1_NMI_INTR_ATTACH(hw_timer_nmi_cb);
    } else {
        ETS_FRC_TIMER1_INTR_ATTACH(hw_timer_isr_cb, NULL);
    }

    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();
}

//-------------------------------Test Code Below--------------------------------------
#if 0
void   hw_test_timer_cb(void)
{
    static uint16 j = 0;
    j++;

    if ((WDEV_NOW() - tick_now2) >= 1000000) {
        static u32 idx = 1;
        tick_now2 = WDEV_NOW();
        os_printf("b%u:%d\n", idx++, j);
        j = 0;
    }

    //hw_timer_arm(50);
}

void ICACHE_FLASH_ATTR user_init(void)
{
    hw_timer_init(FRC1_SOURCE, 1);
    hw_timer_set_func(hw_test_timer_cb);
    hw_timer_arm(100);
}
#endif
/*
NOTE:
1 if use nmi source, for autoload timer , the timer setting val can't be less than 100.
2 if use nmi source, this timer has highest priority, can interrupt other isr.
3 if use frc1 source, this timer can't interrupt other isr.

*/

