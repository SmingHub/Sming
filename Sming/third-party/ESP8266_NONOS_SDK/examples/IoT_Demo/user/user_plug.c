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

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

#include "user_plug.h"

#if PLUG_DEVICE

LOCAL struct plug_saved_param plug_param;
LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[PLUG_KEY_NUM];
LOCAL os_timer_t link_led_timer;
LOCAL uint8 link_led_level = 0;

extern uint32 priv_param_start_sec;     //0x3D

/******************************************************************************
 * FunctionName : user_plug_get_status
 * Description  : get plug's status, 0x00 or 0x01
 * Parameters   : none
 * Returns      : uint8 - plug's status
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
user_plug_get_status(void)
{
    return plug_param.status;
}

/******************************************************************************
 * FunctionName : user_plug_set_status
 * Description  : set plug's status, 0x00 or 0x01
 * Parameters   : uint8 - status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_plug_set_status(bool status)
{
    if (status != plug_param.status) {
        if (status > 1) {
            os_printf("error status input!\n");
            return;
        }

        plug_param.status = status;
        PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, status);
    }
}

/******************************************************************************
 * FunctionName : user_plug_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_plug_short_press(void)
{
    user_plug_set_status((~plug_param.status) & 0x01);

    spi_flash_erase_sector(priv_param_start_sec + PRIV_PARAM_SAVE);
    spi_flash_write((priv_param_start_sec + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
        		(uint32 *)&plug_param, sizeof(struct plug_saved_param));
}

/******************************************************************************
 * FunctionName : user_plug_long_press
 * Description  : key's long press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_plug_long_press(void)
{
	user_esp_platform_set_active(0);
    system_restore();
    system_restart();
}

LOCAL void ICACHE_FLASH_ATTR
user_link_led_init(void)
{
    PIN_FUNC_SELECT(PLUG_LINK_LED_IO_MUX, PLUG_LINK_LED_IO_FUNC);
}

void ICACHE_FLASH_ATTR
user_link_led_output(uint8 level)
{
    GPIO_OUTPUT_SET(GPIO_ID_PIN(PLUG_LINK_LED_IO_NUM), level);
}

LOCAL void ICACHE_FLASH_ATTR
user_link_led_timer_cb(void)
{
    link_led_level = (~link_led_level) & 0x01;
    GPIO_OUTPUT_SET(GPIO_ID_PIN(PLUG_LINK_LED_IO_NUM), link_led_level);
}

void ICACHE_FLASH_ATTR
user_link_led_timer_init(void)
{
    os_timer_disarm(&link_led_timer);
    os_timer_setfn(&link_led_timer, (os_timer_func_t *)user_link_led_timer_cb, NULL);
    os_timer_arm(&link_led_timer, 50, 1);
    link_led_level = 0;
    GPIO_OUTPUT_SET(GPIO_ID_PIN(PLUG_LINK_LED_IO_NUM), link_led_level);
}

void ICACHE_FLASH_ATTR
user_link_led_timer_done(void)
{
    os_timer_disarm(&link_led_timer);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(PLUG_LINK_LED_IO_NUM), 0);
}

/******************************************************************************
 * FunctionName : user_plug_init
 * Description  : init plug's key function and relay output
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_plug_init(void)
{
    user_link_led_init();

    wifi_status_led_install(PLUG_WIFI_LED_IO_NUM, PLUG_WIFI_LED_IO_MUX, PLUG_WIFI_LED_IO_FUNC);

    single_key[0] = key_init_single(PLUG_KEY_0_IO_NUM, PLUG_KEY_0_IO_MUX, PLUG_KEY_0_IO_FUNC,
                                    user_plug_long_press, user_plug_short_press);

    keys.key_num = PLUG_KEY_NUM;
    keys.single_key = single_key;

    key_init(&keys);

    spi_flash_read((priv_param_start_sec + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
        		(uint32 *)&plug_param, sizeof(struct plug_saved_param));

    PIN_FUNC_SELECT(PLUG_RELAY_LED_IO_MUX, PLUG_RELAY_LED_IO_FUNC);

    // no used SPI Flash
    if (plug_param.status == 0xff) {
        plug_param.status = 1;
    }

    PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, plug_param.status);
}
#endif

