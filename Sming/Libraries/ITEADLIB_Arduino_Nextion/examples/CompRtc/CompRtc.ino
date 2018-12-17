/**
 * @example CompRtc.ino
 *
 * @par How to Use
 * This example shows that in nextion screen 
 * displays the current read the RTC time and show how to use the API.
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

NexText t0 = NexText(0,1,"t0");
NexRtc  rtc;
uint32_t  time[7] = {2016,11,25,12,34,50};
uint8_t time_buf[30] = {0};

void setup() 
{
    nexSerial.begin(115200);
    rtc.write_rtc_time(time);
}

void loop() 
{
    rtc.read_rtc_time(time_buf,30);
    t0.setText(time_buf);
    delay(1000);
}
