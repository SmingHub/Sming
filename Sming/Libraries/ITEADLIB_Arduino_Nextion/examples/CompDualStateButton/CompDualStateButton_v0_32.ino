/**
 * @example CompDualStateButton.ino
 * 
 * @par How to Use
 * This example shows that when the dual state button component on the Nextion screen is released,
 * the text of Text component will change one every time.      
 *
 * @author  huang xianming (email:<xianming.huang@itead.cc>)
 * @date    2015/11/11
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

/*
 * Declare a dual state button object [page id:0,component id:1, component name: "bt0"]. 
 */
NexDSButton bt0 = NexDSButton(0, 1, "bt0");

NexText t0 = NexText(0, 2, "t0");


char buffer[100] = {0};

/*
 * Register a dual state button object to the touch event list.  
 */
NexTouch *nex_listen_list[] = 
{
    &bt0,
    NULL
};

/*
 * Dual state button component pop callback function. 
 * In this example,the button's text value will plus one every time when it is released. 
 */
void bt0PopCallback(void *ptr)
{
    uint32_t dual_state;
    NexDSButton *btn = (NexDSButton *)ptr;
    dbSerialPrintln("b0PopCallback");
    dbSerialPrint("ptr=");
    dbSerialPrintln((uint32_t)ptr); 
    memset(buffer, 0, sizeof(buffer));

    /* Get the state value of dual state button component . */
    bt0.getValue(&dual_state);
    if(dual_state) 
    {
        t0.setText("HI! OPEN STATE");
    }
    else
    {
        t0.setText("HI! OFF STATE");
    }
}

void setup(void)
{    
    /* Set the baudrate which is for debug and communicate with Nextion screen. */
    nexInit();

    /* Register the pop event callback function of the dual state button component. */
    bt0.attachPop(bt0PopCallback, &bt0);
    
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












