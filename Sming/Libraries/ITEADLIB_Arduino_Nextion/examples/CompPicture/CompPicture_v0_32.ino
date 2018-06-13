/**
 * @example CompPicture.ino
 *
 * @par How to Use
 * This example shows that ,when the picture component on the Nextion screen is released,
 * the picture of current component will be changed every time.  
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

/*
 * Declare a picture object [page id:0,component id:1, component name: "p0"]. 
 */
NexPicture p0 = NexPicture(0, 1, "p0");

NexTouch *nex_listen_list[] = 
{
    &p0,
    NULL
};

void p0PopCallback(void *ptr)
{
    uint32_t number = 0;
    dbSerialPrintln("p0PopCallback");

    p0.getPic(&number);

    if (number == 1)
    {
        number = 2;    
    }
    else
    {
        number = 1;
    }
    
    p0.setPic(number);
}


void setup(void)
{
    nexInit();
    p0.attachPop(p0PopCallback);
    dbSerialPrintln("setup done");
}

void loop(void)
{
    nexLoop(nex_listen_list);
}

