/**
 * @example CompNumber.ino
 *
 * @par How to Use
 * This example shows that ,when the "+" component on the Nextion screen is released,
 * the value of number component will plus 1,when the "-" component released ,the value of 
 * number component will minus 1 every time.
 *
 * @author  huang xianming (email:<xianming.huang@itead.cc>)
 * @date    2015/11/10
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

void n0PopCallback(void *ptr);
void b0PopCallback(void *ptr);
void b1PopCallback(void *ptr);

/*
 * Declare a number object [page id:0,component id:3, component name: "n0"]. 
 */
NexNumber n0 = NexNumber(0, 3, "n0");

/*
 * Declare a button object [page id:0,component id:1, component name: "b0"]. 
 */
NexButton b0 = NexButton(0, 1, "b0");

/*
 * Declare a button object [page id:0,component id:2, component name: "b1"]. 
 */
NexButton b1 = NexButton(0, 2, "b1");

char buffer[100] = {0};

/*
 * Register object n0, b0, b1, to the touch event list.  
 */
NexTouch *nex_listen_list[] = 
{
    &n0,
    &b0,
    &b1,
    NULL
};

/*
 * number component pop callback function. 
 */
void n0PopCallback(void *ptr)
{
    dbSerialPrintln("n0PopCallback");
    n0.setValue(50);
}

/*
 * Button0 component pop callback function.
 * In this example,the value of the number component will plus one every time when button0 is released.
 */
void b0PopCallback(void *ptr)
{
    uint32_t number;
    
    dbSerialPrintln("b0PopCallback");

    n0.getValue(&number);
    
    number += 1;
    
    n0.setValue(number);
}

/*
 * Button1 component pop callback function.
 * In this example,the value of the number component will minus one every time when button1 is released.
 */
void b1PopCallback(void *ptr)
{
     uint32_t number;
    
    dbSerialPrintln("b1PopCallback");

    n0.getValue(&number);
    
    number -= 1;
    
    n0.setValue(number);
}

void setup(void)
{
    /* Set the baudrate which is for debug and communicate with Nextion screen. */
    nexInit();

    /* Register the pop event callback function of the current number component. */
    n0.attachPop(n0PopCallback);

    /* Register the pop event callback function of the current button0 component. */
    b0.attachPop(b0PopCallback);

    /* Register the pop event callback function of the current button1 component. */
    b1.attachPop(b1PopCallback);

    dbSerialPrintln("setup done");
}

void loop(void)
{
    /*
     * When a pop or push event occured every time, 
     * the corresponding component[right page id and component id] in touch event list will be asked.
     */
    nexLoop(nex_listen_list);
}


