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
#include "at_custom.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"


static struct espconn *at_espconn_demo_espconn_ptr = NULL;

#define AT_ESPCONN_DEMO_BUFFER_SIZE 	(2920)
static uint8 at_espconn_demo_buffer[AT_ESPCONN_DEMO_BUFFER_SIZE];
static uint32 at_espconn_demo_data_len = 0;
static bool at_espconn_demo_flag = FALSE;

static void ICACHE_FLASH_ATTR
at_espconn_demo_recon_cb(void *arg, sint8 errType)
{
	struct espconn *espconn_ptr = (struct espconn *)arg;

	os_printf("at demo espconn reconnect\r\n");
	at_espconn_demo_flag = FALSE;
    espconn_connect(espconn_ptr);
}


// notify at module that espconn has received data
static void ICACHE_FLASH_ATTR
at_espconn_demo_recv(void *arg, char *pusrdata, unsigned short len)
{
	at_fake_uart_rx(pusrdata,len);
}

static void ICACHE_FLASH_ATTR
at_espconn_demo_send_cb(void *arg)
{
	at_espconn_demo_flag = TRUE;
	if(at_espconn_demo_data_len) {
		espconn_send(at_espconn_demo_espconn_ptr,at_espconn_demo_buffer,at_espconn_demo_data_len);
		at_espconn_demo_data_len = 0;
	}
}
static void ICACHE_FLASH_ATTR
at_espconn_demo_discon_cb(void *arg)
{
  struct espconn *espconn_ptr = (struct espconn *)arg;

  os_printf("at demo espconn disconnected\r\n");
  at_espconn_demo_flag = FALSE;
  espconn_connect(espconn_ptr);
}

static void ICACHE_FLASH_ATTR
at_espconn_demo_connect_cb(void *arg)
{
	os_printf("at demo espconn connected\r\n");
	espconn_set_opt((struct espconn*)arg,ESPCONN_COPY);
	at_espconn_demo_flag = TRUE;
	at_espconn_demo_data_len = 0;
}

static void ICACHE_FLASH_ATTR at_espconn_demo_response(const uint8*data,uint32 length)
{
	if((data == NULL) || (length == 0)) {
		return;
	}

	if(at_espconn_demo_flag) {
		espconn_send(at_espconn_demo_espconn_ptr,(uint8*)data,length);
		at_espconn_demo_flag = FALSE;
	} else {
		if(length <= (AT_ESPCONN_DEMO_BUFFER_SIZE - at_espconn_demo_data_len)) {
			os_memcpy(at_espconn_demo_buffer + at_espconn_demo_data_len,data,length);
			at_espconn_demo_data_len += length;
		} else {
			os_printf("at espconn buffer full\r\n");
		}
	}
}


static void ICACHE_FLASH_ATTR at_espconn_demo_init(void)
{
  uint32 ip = 0;
  at_espconn_demo_espconn_ptr = (struct espconn *)os_zalloc(sizeof(struct espconn));
  at_espconn_demo_espconn_ptr->type = ESPCONN_TCP;
  at_espconn_demo_espconn_ptr->state = ESPCONN_NONE;
  at_espconn_demo_espconn_ptr->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  at_espconn_demo_espconn_ptr->proto.tcp->local_port = espconn_port();
  at_espconn_demo_espconn_ptr->proto.tcp->remote_port = 8999;

  ip = ipaddr_addr("192.168.1.120");
  os_memcpy(at_espconn_demo_espconn_ptr->proto.tcp->remote_ip,&ip,sizeof(ip));
  espconn_regist_connectcb(at_espconn_demo_espconn_ptr, at_espconn_demo_connect_cb);
  espconn_regist_reconcb(at_espconn_demo_espconn_ptr, at_espconn_demo_recon_cb);
  espconn_regist_disconcb(at_espconn_demo_espconn_ptr, at_espconn_demo_discon_cb);
  espconn_regist_recvcb(at_espconn_demo_espconn_ptr, at_espconn_demo_recv);
  espconn_regist_sentcb(at_espconn_demo_espconn_ptr, at_espconn_demo_send_cb);
  
  espconn_connect(at_espconn_demo_espconn_ptr);
  
  at_fake_uart_enable(TRUE,at_espconn_demo_response);
}

static void ICACHE_FLASH_ATTR
at_exeCmdTest(uint8_t id)
{
	at_response_ok();
	at_espconn_demo_init();
}

extern void at_exeCmdCiupdate(uint8_t id);
at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, NULL, NULL, NULL, at_exeCmdTest},
#ifdef AT_UPGRADE_SUPPORT
    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
};

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
    system_phy_freq_trace_enable(at_get_rf_auto_trace_from_flash());
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    char buf[64] = {0};
    at_customLinkMax = 5;
    at_init();
    os_sprintf(buf,"compile time:%s %s",__DATE__,__TIME__);
    at_set_custom_info(buf);
    at_port_print("\r\nready\r\n");
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]));
	at_port_print("\r\n***==================================***");
	at_port_print("\r\n***  Welcome to at espconn demo!!!   ***");
	at_port_print("\r\n*** Please create a TCP Server on PC,***");
	at_port_print("\r\n*** then enter command AT+TEST.      ***");
	at_port_print("\r\n***==================================***\r\n");
}
