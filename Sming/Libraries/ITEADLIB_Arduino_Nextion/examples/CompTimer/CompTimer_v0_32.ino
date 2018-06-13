/**
 * @example CompTimer.ino
 * 
 * @par How to Use
 * This example shows that ,when the OFF button component on the Nextion screen is released,
 * the timer will opened,the text will show number changed and push the ADDTIME button timer 
 * cycle value will increase,when push the DECTIME button timer cycle  value will reduce.
 *
 * @author huang xianming (email:<xianming.huang@itead.cc>)
 * @date    2015/8/25
 * @updated 2016/12/25 bring HMI up to v0.32 to avoid too old issues
 * @convert by Patrick Martin, no other changes made
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include "Nextion.h"

NexButton b0 = NexButton(0, 2, "b0");
NexButton b1 = NexButton(0, 5, "b1");
NexButton b2 = NexButton(0, 6, "b2");
NexText t0 = NexText(0, 3, "t0");
NexText t1 = NexText(0, 4, "t1");
NexTimer tm0 = NexTimer(0, 1, "tm0");


char buffer[100] = {0};
uint32_t number_timer = 0;
uint32_t number_enable = 0;
uint32_t number_cycle = 100;



NexTouch *nex_listen_list[] = 
{
    &b0,
    &b1,
    &b2,
    &t0,
    &t1,
    &tm0,
    NULL
};
/*
 * Button component pop callback function. 
 * In this example,the button can open the timer when it is released.
 */
void b0PopCallback(void *ptr)
{
    if(number_enable == 1)
    {
        tm0.enable();
        number_enable = 0;
        b0.setText("ON");
    }
    else if (number_enable ==0)
    {
        tm0.disable();
        number_enable =1;
        b0.setText("OFF");
    }
}
/*
 * Button component pop callback function. 
 * In this example,the timer's cycle value will increase when it is released. 
 */
void b1PopCallback(void *ptr)
{
    tm0.getCycle(&number_cycle);
    number_cycle = number_cycle + 100;
    tm0.setCycle(number_cycle);
    memset(buffer, 0, sizeof(buffer));
    itoa(number_cycle, buffer, 10);
    t1.setText(buffer);
}

/*
 * Button component pop callback function. 
 * In this example,the timer's cycle value will reduce when it is released. 
 */

void b2PopCallback(void *ptr)
{
    tm0.getCycle(&number_cycle);
    if (number_cycle >100)
    {
        number_cycle = number_cycle - 100;
    }
    tm0.setCycle(number_cycle);
    memset(buffer, 0, sizeof(buffer));
    itoa(number_cycle, buffer, 10);
    t1.setText(buffer);
}

/*
 * The timer respond function 
 * In this example,the timer will respond when set cycle time done and puls one for a variable. 
 */

void tm0TimerCallback(void *ptr)
{
    number_timer++;
    memset(buffer, 0, sizeof(buffer));
    itoa(number_timer, buffer, 10);
    t0.setText(buffer);
}
void setup(void)
{    
    nexInit();
    b0.attachPop(b0PopCallback);
    tm0.attachTimer(tm0TimerCallback);
    b1.attachPop(b1PopCallback);
    b2.attachPop(b2PopCallback);
    dbSerialPrintln("setup done"); 
}

void loop(void)
{   
    nexLoop(nex_listen_list);
}

