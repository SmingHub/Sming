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

#include "osapi.h"
#include "user_interface.h"

#include "wpa2_enterprise.h"

#include "wpa2_pki/ca.h"
#include "wpa2_pki/client_crt.h"
#include "wpa2_pki/client_key.h"

#define SSID "wpa2_test"
#define PASSWORD ""

#define WPA2_IDENTITY "user1"
#define WPA2_USERNAME "espressif"
#define WPA2_PASSWORD "test11"

typedef enum {
    EAP_TLS,
    EAP_PEAP,
    EAP_TTLS,
} eap_method_t;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

void user_set_station_config(void)
{
    char ssid[32] = SSID;
    char password[64] = PASSWORD;
    struct station_config sta_conf = { 0 };

    os_memcpy(sta_conf.ssid, ssid, 32);
    os_memcpy(sta_conf.password, password, 64);
    wifi_station_set_config(&sta_conf);
}

void user_set_wpa2_config(void)
{
    eap_method_t method = EAP_TLS;
    char *identity = WPA2_IDENTITY;
    char *username = WPA2_USERNAME;
    char *password = WPA2_PASSWORD;

    wifi_station_set_wpa2_enterprise_auth(1);

    //wifi_station_set_enterprise_identity(identity, os_strlen(identity));//This is an option. If not call this API, the outer identity will be "anonymous@espressif.com".

    if (method == EAP_TLS) {
        wifi_station_set_enterprise_cert_key(client_cert, os_strlen(client_cert)+1, client_key, os_strlen(client_key)+1, NULL, 1);
        //wifi_station_set_enterprise_username(username, os_strlen(username));//This is an option for EAP_PEAP and EAP_TLS.
    }
    else if (method == EAP_PEAP || method == EAP_TTLS) {
        wifi_station_set_enterprise_username(username, os_strlen(username));
        wifi_station_set_enterprise_password(password, os_strlen(password));
        //wifi_station_set_enterprise_ca_cert(ca, os_strlen(ca)+1);//This is an option for EAP_PEAP and EAP_TTLS.
    }
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    os_printf("This is an example for wpa2 enterprise...\n");
    
    wifi_set_opmode(STATION_MODE);
    user_set_station_config();
    user_set_wpa2_config();
    wifi_station_connect();
}
