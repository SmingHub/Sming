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

#ifndef __USER_IOT_VERSION_H__
#define __USER_IOT_VERSION_H__

#include "user_config.h"

#define IOT_VERSION_MAJOR		1U
#define IOT_VERSION_MINOR		0U
#define IOT_VERSION_REVISION	5U

#define VERSION_NUM   (IOT_VERSION_MAJOR * 1000 + IOT_VERSION_MINOR * 100 + IOT_VERSION_REVISION)

//#define VERSION_TYPE      "b"
#define VERSION_TYPE   	  "v"

#if LIGHT_DEVICE
#define device_type       45772
#elif PLUG_DEVICE
#define device_type       23701
#elif SENSOR_DEVICE
#define device_type       12335
#endif


#define ONLINE_UPGRADE    0
#define LOCAL_UPGRADE     0
#define ALL_UPGRADE       1
#define NONE_UPGRADE      0

#if	ONLINE_UPGRADE
#define UPGRADE_FALG	"O"
#elif  LOCAL_UPGRADE
#define UPGRADE_FALG	"l"
#elif  ALL_UPGRADE
#define UPGRADE_FALG	"a"
#elif NONE_UPGRADE
#define UPGRADE_FALG	"n"
#endif

#define IOT_VERSION


#endif

