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


#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 3)
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 4)
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0xE0000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#define SYSTEM_PARTITION_AT_PARAMETER_ADDR					0xfd000
#define SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY_ADDR		0xfc000
#define SYSTEM_PARTITION_SSL_CLIENT_CA_ADDR					0xfb000
#define SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY_ADDR	0xfa000
#define SYSTEM_PARTITION_WPA2_ENTERPRISE_CA_ADDR			0xf9000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0xE0000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#define SYSTEM_PARTITION_AT_PARAMETER_ADDR					0xfd000
#define SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY_ADDR		0xfc000
#define SYSTEM_PARTITION_SSL_CLIENT_CA_ADDR					0xfb000
#define SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY_ADDR	0xfa000
#define SYSTEM_PARTITION_WPA2_ENTERPRISE_CA_ADDR			0xf9000
#else
#error "The flash map is not supported"
#endif


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

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
    { SYSTEM_PARTITION_AT_PARAMETER, 					SYSTEM_PARTITION_AT_PARAMETER_ADDR, 				0x3000},
	{ SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY, 		SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY_ADDR, 		0x1000},
	{ SYSTEM_PARTITION_SSL_CLIENT_CA, 					SYSTEM_PARTITION_SSL_CLIENT_CA_ADDR, 				0x1000},
#ifdef CONFIG_AT_WPA2_ENTERPRISE_COMMAND_ENABLE
	{ SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY, 	SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY_ADDR,	0x1000},
    { SYSTEM_PARTITION_WPA2_ENTERPRISE_CA, 				SYSTEM_PARTITION_WPA2_ENTERPRISE_CA_ADDR, 			0x1000},
#endif
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
    system_phy_freq_trace_enable(at_get_rf_auto_trace_from_flash());
}

#ifdef CONFIG_ENABLE_IRAM_MEMORY
uint32 user_iram_memory_is_enabled(void)
{
    return CONFIG_ENABLE_IRAM_MEMORY;
}
#endif

void ICACHE_FLASH_ATTR
user_init(void)
{
    char buf[128] = {0};
    at_customLinkMax = 5;
    at_init();
#ifdef ESP_AT_FW_VERSION
    if ((ESP_AT_FW_VERSION != NULL) && (os_strlen(ESP_AT_FW_VERSION) < 64)) {
        os_sprintf(buf,"compile time:"__DATE__" "__TIME__"\r\n"ESP_AT_FW_VERSION);
    } else {
        os_sprintf(buf,"compile time:"__DATE__" "__TIME__);
    }
#else
    os_sprintf(buf,"compile time:"__DATE__" "__TIME__);
#endif
    at_set_custom_info(buf);
    at_port_print_irom_str("\r\nready\r\n");
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]));
#ifdef CONFIG_AT_SMARTCONFIG_COMMAND_ENABLE
    at_cmd_enable_smartconfig();
#endif
#ifdef CONFIG_AT_WPA2_ENTERPRISE_COMMAND_ENABLE
    at_cmd_enable_wpa2_enterprise();
#endif
	at_port_print("\r\n***==================================***");
	at_port_print("\r\n***  Welcome to at espconn demo!!!   ***");
	at_port_print("\r\n*** Please create a TCP Server on PC,***");
	at_port_print("\r\n*** then enter command AT+TEST.      ***");
	at_port_print("\r\n***==================================***\r\n");
}
