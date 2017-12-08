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


#include "simple_pair.h"

/*******************************************************************************
 * open AS_STA to compile sta test code
 * open AS_AP to compile ap test code
 * don't open both
 * ****************************************************************************/
#define AS_STA
//#define AS_AP

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
/* STA & AP use the same tmpkey to encrypt Simple Pair communication */
static u8 tmpkey[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

#ifdef AS_STA
/* since the ex_key transfer from AP to STA, so STA's ex_key don't care */
static u8 ex_key[16] = {0x00};
#endif /* AS_STA */

#ifdef AS_AP
/* since the ex_key transfer from AP to STA, so AP's ex_key must be set */
static u8 ex_key[16] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
			0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
#endif /* AS_AP */

void ICACHE_FLASH_ATTR
show_key(u8 *buf, u8 len)
{
	u8 i;

	for (i = 0; i < len; i++)
		os_printf("%02x,%s", buf[i], (i%16 == 15?"\n":" "));
}

#ifdef AS_STA
static void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{
    int ret;

    if (status == OK) {

        struct bss_info *bss_link = (struct bss_info *)arg;

        while (bss_link != NULL) {
	    if (bss_link->simple_pair) {
                os_printf("Simple Pair: bssid %02x:%02x:%02x:%02x:%02x:%02x Ready!\n", 
				bss_link->bssid[0], bss_link->bssid[1], bss_link->bssid[2],
				bss_link->bssid[3], bss_link->bssid[4], bss_link->bssid[5]);
		simple_pair_set_peer_ref(bss_link->bssid, tmpkey, NULL);
		ret = simple_pair_sta_start_negotiate();
		if (ret)
			os_printf("Simple Pair: STA start NEG Failed\n");
		else
			os_printf("Simple Pair: STA start NEG OK\n");
		break;
	    }
            bss_link = bss_link->next.stqe_next;
        }
    } else {
        os_printf("err, scan status %d\n", status);
    }

}
#endif


void ICACHE_FLASH_ATTR
sp_status(u8 *sa, u8 status)
{
#ifdef AS_STA
	switch (status) {
	case  SP_ST_STA_FINISH:
		simple_pair_get_peer_ref(NULL, NULL, ex_key);
		os_printf("Simple Pair: STA FINISH, Ex_key ");
		show_key(ex_key, 16);
		/* TODO: Try to use the ex-key communicate with AP, for example use ESP-NOW */

		/* if test ok , deinit simple pair */
		simple_pair_deinit();
		break;
	case SP_ST_STA_AP_REFUSE_NEG:
		/* AP refuse , so try simple pair again  or scan other ap*/
		os_printf("Simple Pair: Recv AP Refuse\n");
		simple_pair_state_reset();
		simple_pair_sta_enter_scan_mode();
		wifi_station_scan(NULL, scan_done);
		break;
	case SP_ST_WAIT_TIMEOUT:
		/* In negotiate, timeout , so try simple pair again */
		os_printf("Simple Pair: Neg Timeout\n");
		simple_pair_state_reset();
		simple_pair_sta_enter_scan_mode();
		wifi_station_scan(NULL, scan_done);
		break;
	case SP_ST_SEND_ERROR:
		os_printf("Simple Pair: Send Error\n");
		/* maybe the simple_pair_set_peer_ref() haven't called, it send to a wrong mac address */

		break;
	case SP_ST_KEY_INSTALL_ERR:
		os_printf("Simple Pair: Key Install Error\n");
		/* 1. maybe something argument error.
 		   2. maybe the key number is full in system*/

		/* TODO: Check other modules which use lots of keys 
                         Example: ESPNOW and STA/AP use lots of keys */
		break;
	case SP_ST_KEY_OVERLAP_ERR:
		os_printf("Simple Pair: Key Overlap Error\n");
		/* 1. maybe something argument error.
 		   2. maybe the MAC Address is already use in ESP-NOW or other module
		      the same MAC Address has multi key*/

		/* TODO: Check if the same MAC Address used already,
                         Example: del MAC item of ESPNOW or other module */
		break;
	case SP_ST_OP_ERROR:
		os_printf("Simple Pair: Operation Order Error\n");
		/* 1. maybe the function call order has something wrong */

		/* TODO: Adjust your function call order */
		break;
	default:
		os_printf("Simple Pair: Unknown Error\n");
		break;
	}
		
#endif /* AS_STA */

#ifdef AS_AP
	switch (status) {
	case  SP_ST_AP_FINISH:
		simple_pair_get_peer_ref(NULL, NULL, ex_key);
		os_printf("Simple Pair: AP FINISH\n");

		/* TODO: Wait STA use the ex-key communicate with AP, for example use ESP-NOW */
		
		/* if test ok , deinit simple pair */
		simple_pair_deinit();
		break;
	case SP_ST_AP_RECV_NEG:
		/* AP recv a STA's negotiate request */
		os_printf("Simple Pair: Recv STA Negotiate Request\n");

		/* set peer must be called, because the simple pair need to know what peer mac is */
		simple_pair_set_peer_ref(sa, tmpkey, ex_key);

		/* TODO:In this phase, the AP can interaction with Smart Phone,
                   if the Phone agree, call start_neg or refuse */
		simple_pair_ap_start_negotiate();
		//simple_pair_ap_refuse_negotiate();
		/* TODO:if refuse, maybe call simple_pair_deinit() to ending the simple pair */

		break;
	case SP_ST_WAIT_TIMEOUT:
		/* In negotiate, timeout , so re-enter in to announce mode*/
		os_printf("Simple Pair: Neg Timeout\n");
		simple_pair_state_reset();
		simple_pair_ap_enter_announce_mode();
		break;
	case SP_ST_SEND_ERROR:
		os_printf("Simple Pair: Send Error\n");
		/* maybe the simple_pair_set_peer_ref() haven't called, it send to a wrong mac address */

		break;
	case SP_ST_KEY_INSTALL_ERR:
		os_printf("Simple Pair: Key Install Error\n");
		/* 1. maybe something argument error.
 		   2. maybe the key number is full in system*/

		/* TODO: Check other modules which use lots of keys 
                         Example: ESPNOW and STA/AP use lots of keys */
		break;
	case SP_ST_KEY_OVERLAP_ERR:
		os_printf("Simple Pair: Key Overlap Error\n");
		/* 1. maybe something argument error.
 		   2. maybe the MAC Address is already use in ESP-NOW or other module
		      the same MAC Address has multi key*/

		/* TODO: Check if the same MAC Address used already,
                         Example: del MAC item of ESPNOW or other module */
		break;
	case SP_ST_OP_ERROR:
		os_printf("Simple Pair: Operation Order Error\n");
		/* 1. maybe the function call order has something wrong */

		/* TODO: Adjust your function call order */
		break;
	default:
		os_printf("Simple Pair: Unknown Error\n");
		break;
	}
	
#endif /* AS_AP */
}

void ICACHE_FLASH_ATTR
init_done(void)
{
	int ret;

#ifdef AS_STA
	wifi_set_opmode(STATION_MODE);

	/* init simple pair */
	ret = simple_pair_init();
	if (ret) {
		os_printf("Simple Pair: init error, %d\n", ret);
		return;
	}
	/* register simple pair status callback function */
	ret = register_simple_pair_status_cb(sp_status);
	if (ret) {
		os_printf("Simple Pair: register status cb error, %d\n", ret);
		return;
	}

	os_printf("Simple Pair: STA Enter Scan Mode ...\n");
	ret = simple_pair_sta_enter_scan_mode();
	if (ret) {
		os_printf("Simple Pair: STA Enter Scan Mode Error, %d\n", ret);
		return;
	}
	/* scan ap to searh which ap is ready to simple pair */
	os_printf("Simple Pair: STA Scan AP ...\n");
        wifi_station_scan(NULL,scan_done);
#endif
#ifdef AS_AP
	wifi_set_opmode(SOFTAP_MODE);

	/* init simple pair */
	ret = simple_pair_init();
	if (ret) {
		os_printf("Simple Pair: init error, %d\n", ret);
		return;
	}
	/* register simple pair status callback function */
	ret = register_simple_pair_status_cb(sp_status);
	if (ret) {
		os_printf("Simple Pair: register status cb error, %d\n", ret);
		return;
	}

	os_printf("Simple Pair: AP Enter Announce Mode ...\n");
	/* ap must enter announce mode , so the sta can know which ap is ready to simple pair */
	ret = simple_pair_ap_enter_announce_mode();
	if (ret) {
		os_printf("Simple Pair: AP Enter Announce Mode Error, %d\n", ret);
		return;
	}

#endif

}


void ICACHE_FLASH_ATTR
user_init(void)
{
	system_init_done_cb(init_done);
}
