/**
 * @example CompPage.ino
 *
 * @par How to Use
 * Show how to use API of class NexPage.  
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

NexPage page0    = NexPage(0, 0, "page0");
NexPage page1    = NexPage(1, 0, "page1");
NexPage page2    = NexPage(2, 0, "page2");
NexPage page3    = NexPage(3, 0, "page3");

NexTouch *nex_listen_list[] = 
{
    &page0,
    &page1,
    &page2,
    &page3,
    NULL
};

void page0PopCallback(void *ptr)
{
    dbSerialPrintln("page0PopCallback");
    page1.show();
}

void page1PopCallback(void *ptr)
{
    dbSerialPrintln("page1PopCallback");
    page2.show();
}

void page2PopCallback(void *ptr)
{
    dbSerialPrintln("page2PopCallback");
    page3.show();
}

void page3PopCallback(void *ptr)
{
    dbSerialPrintln("page3PopCallback");
    page0.show();
}

void setup(void)
{   
    nexInit();
    dbSerialPrintln("setup begin");
    
    page0.attachPop(page0PopCallback);
    page1.attachPop(page1PopCallback);
    page2.attachPop(page2PopCallback);
    page3.attachPop(page3PopCallback);
    
    dbSerialPrintln("setup end");
}

void loop(void)
{
    nexLoop(nex_listen_list);
}
