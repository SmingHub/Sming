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

#ifndef __USER_SENSOR_H__
#define __USER_SENSOR_H__

#include "user_config.h"
#include "driver/key.h"

#define SENSOR_KEY_NUM    1

#define SENSOR_KEY_IO_MUX     PERIPHS_IO_MUX_MTCK_U
#define SENSOR_KEY_IO_NUM     13
#define SENSOR_KEY_IO_FUNC    FUNC_GPIO13

#define SENSOR_WIFI_LED_IO_MUX     PERIPHS_IO_MUX_GPIO0_U
#define SENSOR_WIFI_LED_IO_NUM     0
#define SENSOR_WIFI_LED_IO_FUNC    FUNC_GPIO0

#define SENSOR_LINK_LED_IO_MUX     PERIPHS_IO_MUX_MTDI_U
#define SENSOR_LINK_LED_IO_NUM     12
#define SENSOR_LINK_LED_IO_FUNC    FUNC_GPIO12

#define SENSOR_UNUSED_LED_IO_MUX     PERIPHS_IO_MUX_MTDO_U
#define SENSOR_UNUSED_LED_IO_NUM     15
#define SENSOR_UNUSED_LED_IO_FUNC    FUNC_GPIO15

#if HUMITURE_SUB_DEVICE
bool user_mvh3004_read_th(uint8 *data);
void user_mvh3004_init(void);
#endif

void user_sensor_init(uint8 active);

#endif
