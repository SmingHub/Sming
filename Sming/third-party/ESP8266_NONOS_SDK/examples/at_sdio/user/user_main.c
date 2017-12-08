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

#include "driver/sdio_slv.h"

#ifdef SDIO_DEBUG
static os_timer_t at_spi_check;
uint32 sum_len = 0;
extern void ICACHE_FLASH_ATTR at_spi_check_cb(void *arg);
#endif

uint32 at_fake_uart_rx(uint8* data,uint32 length);
typedef void (*at_fake_uart_tx_func_type)(const uint8*data,uint32 length);
bool at_fake_uart_enable(bool enable,at_fake_uart_tx_func_type at_fake_uart_tx_func);

typedef void (*at_custom_uart_rx_buffer_fetch_cb_type)(void);
void at_register_uart_rx_buffer_fetch_cb(at_custom_uart_rx_buffer_fetch_cb_type rx_buffer_fetch_cb);
// test :AT+TEST=1,"abc"<,3>
void ICACHE_FLASH_ATTR
at_setupCmdTest(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    uint8 buffer[32] = {0};
    pPara++; // skip '='

    //get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        at_response_error();
        return;
    }

    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    os_sprintf(buffer, "the first parameter:%d\r\n", result);
    at_port_print(buffer);

    //get the second parameter
    // string
    at_data_str_copy(buffer, &pPara, 10);
    at_port_print("the second parameter:");
    at_port_print(buffer);
    at_port_print("\r\n");

    if (*pPara == ',') {
        pPara++; // skip ','
        result = 0;
        //there is the third parameter
        // digit
        flag = at_get_next_int_dec(&pPara, &result, &err);
        // we donot care of flag
        os_sprintf(buffer, "the third parameter:%d\r\n", result);
        at_port_print(buffer);
    }

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_testCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_testCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_queryCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_queryCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_exeCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_exeCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

extern void at_exeCmdCiupdate(uint8_t id);
at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, at_testCmdTest, at_queryCmdTest, at_setupCmdTest, at_exeCmdTest},
#ifdef AT_UPGRADE_SUPPORT
    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
};

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
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

void ICACHE_FLASH_ATTR at_sdio_response(const uint8*data,uint32 length)
{
	if((data == NULL) || (length == 0)) {
		return;
	}

	sdio_load_data(data,length);
}

uint32 sdio_recv_data_callback(uint8* data,uint32 len)
{
	return at_fake_uart_rx(data,len);
}

extern void at_custom_uart_rx_buffer_fetch_cb(void);

void ICACHE_FLASH_ATTR user_init(void)
{
    char buf[64] = {0};
    at_customLinkMax = 5;
	sdio_slave_init();
	sdio_register_recv_cb(sdio_recv_data_callback);
    at_init();
	at_register_uart_rx_buffer_fetch_cb(at_custom_uart_rx_buffer_fetch_cb);
    os_sprintf(buf,"compile time:%s %s",__DATE__,__TIME__);
    at_set_custom_info(buf);
	at_fake_uart_enable(TRUE,at_sdio_response);
	
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]));

	espconn_tcp_set_wnd(4);
	at_port_print("\r\nready\r\n");

#ifdef SDIO_DEBUG
	os_timer_disarm(&at_spi_check);
	os_timer_setfn(&at_spi_check, (os_timer_func_t *)at_spi_check_cb, NULL);
	os_timer_arm(&at_spi_check, 1000, 1);
	os_printf("\r\ntimer start\r\n");
#endif
}
