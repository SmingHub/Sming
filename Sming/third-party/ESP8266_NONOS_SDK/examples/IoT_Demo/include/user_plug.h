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

#ifndef __USER_ESPSWITCH_H__
#define __USER_ESPSWITCH_H__

#include "driver/key.h"

#define PRIV_PARAM_SAVE     0

#define PLUG_KEY_NUM            1

#define PLUG_KEY_0_IO_MUX     PERIPHS_IO_MUX_MTCK_U
#define PLUG_KEY_0_IO_NUM     13
#define PLUG_KEY_0_IO_FUNC    FUNC_GPIO13

#define PLUG_WIFI_LED_IO_MUX     PERIPHS_IO_MUX_GPIO0_U
#define PLUG_WIFI_LED_IO_NUM     0
#define PLUG_WIFI_LED_IO_FUNC    FUNC_GPIO0

#define PLUG_LINK_LED_IO_MUX     PERIPHS_IO_MUX_MTDI_U
#define PLUG_LINK_LED_IO_NUM     12
#define PLUG_LINK_LED_IO_FUNC    FUNC_GPIO12

#define PLUG_RELAY_LED_IO_MUX     PERIPHS_IO_MUX_MTDO_U
#define PLUG_RELAY_LED_IO_NUM     15
#define PLUG_RELAY_LED_IO_FUNC    FUNC_GPIO15

#define PLUG_STATUS_OUTPUT(pin, on)     GPIO_OUTPUT_SET(pin, on)

struct plug_saved_param {
    uint8_t status;
    uint8_t pad[3];
};

void user_plug_init(void);
uint8 user_plug_get_status(void);
void user_plug_set_status(bool status);


#endif

