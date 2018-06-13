/**
 * @example CompProgressBar.ino
 *
 * @par How to Use
 * This example shows that,when the "btn_up" component on the Nextion screen is released,
 * the value of progress bar component will plus 5,when the "btn_down" component released ,the value of 
 * progress bar component will minus 5 every time.
 *
 * @author  Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date    2015/7/10
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

NexProgressBar j0  = NexProgressBar(0, 3, "j0");
NexButton btn_up   = NexButton(0, 1, "btn_up");
NexButton btn_down = NexButton(0, 2, "btn_down");

NexTouch *nex_listen_list[] = 
{
    &btn_up,
    &btn_down,
    NULL
};

void buttonUpPopCallback(void *ptr)
{
    uint32_t number = 0;
    dbSerialPrintln("buttonUpPopCallback");

    j0.getValue(&number);

    number += 5;
    if (number >= 100)
    {
        number = 100;
    }
    
    j0.setValue(number);
}

void buttonDownPopCallback(void *ptr)
{
    uint32_t number = 0;
    dbSerialPrintln("buttonDownPopCallback");

    j0.getValue(&number);

    if (number >= 5)
    {
        number -= 5;
    }
    
    j0.setValue(number);
}

void setup(void)
{
    nexInit();
    btn_up.attachPop(buttonUpPopCallback);
    btn_down.attachPop(buttonDownPopCallback);
    dbSerialPrintln("setup done");
}

void loop(void)
{
    nexLoop(nex_listen_list);
}

