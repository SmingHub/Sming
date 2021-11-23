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

/*
 * Contains definitions from `user_interface.h` which do not relate to WiFi/networking.
 */

#pragma once

#include "os_type.h"
#include "osapi.h"
#include "ets_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

enum rst_reason {
	REASON_DEFAULT_RST = 0,
	REASON_WDT_RST = 1,
	REASON_EXCEPTION_RST = 2,
	REASON_SOFT_WDT_RST = 3,
	REASON_SOFT_RESTART = 4,
	REASON_DEEP_SLEEP_AWAKE = 5,
	REASON_EXT_SYS_RST = 6,
};

struct rst_info {
	uint32_t reason;
	uint32_t exccause;
	uint32_t epc1;
	uint32_t epc2;
	uint32_t epc3;
	uint32_t excvaddr;
	uint32_t depc;
};

struct rst_info* system_get_rst_info(void);

#define UPGRADE_FW_BIN1 0x00
#define UPGRADE_FW_BIN2 0x01

void system_restore(void);
void system_restart(void);

bool system_deep_sleep_set_option(uint8_t option);
bool system_deep_sleep(uint64_t time_in_us);
bool system_deep_sleep_instant(uint64_t time_in_us);

uint8_t system_upgrade_userbin_check(void);
void system_upgrade_reboot(void);
uint8_t system_upgrade_flag_check();
void system_upgrade_flag_set(uint8_t flag);

void system_timer_reinit(void);
uint32_t system_get_time(void);

/* user task's prio must be 0/1/2 !!!*/
enum {
	USER_TASK_PRIO_0 = 0,
	USER_TASK_PRIO_1,
	USER_TASK_PRIO_2,
	USER_TASK_PRIO_MAX,
};

bool system_os_task(os_task_t task, uint8_t prio, os_event_t* queue, uint8_t qlen);
bool system_os_post(uint8_t prio, os_signal_t sig, os_param_t par);

void system_print_meminfo(void);
uint32_t system_get_free_heap_size(void);

void system_set_os_print(uint8_t onoff);
uint8_t system_get_os_print();

uint64_t system_mktime(uint32_t year, uint32_t mon, uint32_t day, uint32_t hour, uint32_t min, uint32_t sec);

uint32_t system_get_chip_id(void);

typedef void (*init_done_cb_t)(void);

void system_init_done_cb(init_done_cb_t cb);

uint32_t system_rtc_clock_cali_proc(void);
uint32_t system_get_rtc_time(void);

bool system_rtc_mem_read(uint8_t src_addr, void* des_addr, uint16_t load_size);
bool system_rtc_mem_write(uint8_t des_addr, const void* src_addr, uint16_t save_size);

void system_uart_swap(void);
void system_uart_de_swap(void);

uint16_t system_adc_read(void);
void system_adc_read_fast(uint16_t* adc_addr, uint16_t adc_num, uint8_t adc_clk_div);
uint16_t system_get_vdd33(void);

const char* system_get_sdk_version(void);

#define SYS_BOOT_ENHANCE_MODE 0
#define SYS_BOOT_NORMAL_MODE 1

#define SYS_BOOT_NORMAL_BIN 0
#define SYS_BOOT_TEST_BIN 1

uint8_t system_get_boot_version(void);
uint32_t system_get_userbin_addr(void);
uint8_t system_get_boot_mode(void);
bool system_restart_enhance(uint8_t bin_type, uint32_t bin_addr);

#define SYS_CPU_80MHZ 80
#define SYS_CPU_160MHZ 160

bool system_update_cpu_freq(uint8_t freq);
uint8_t system_get_cpu_freq(void);

enum flash_size_map {
	FLASH_SIZE_4M_MAP_256_256 = 0, /**<  Flash size : 4Mbits. Map : 256KBytes + 256KBytes */
	FLASH_SIZE_2M,				   /**<  Flash size : 2Mbits. Map : 256KBytes */
	FLASH_SIZE_8M_MAP_512_512,	 /**<  Flash size : 8Mbits. Map : 512KBytes + 512KBytes */
	FLASH_SIZE_16M_MAP_512_512,	/**<  Flash size : 16Mbits. Map : 512KBytes + 512KBytes */
	FLASH_SIZE_32M_MAP_512_512,	/**<  Flash size : 32Mbits. Map : 512KBytes + 512KBytes */
	FLASH_SIZE_16M_MAP_1024_1024,  /**<  Flash size : 16Mbits. Map : 1024KBytes + 1024KBytes */
	FLASH_SIZE_32M_MAP_1024_1024,  /**<  Flash size : 32Mbits. Map : 1024KBytes + 1024KBytes */
	FLASH_SIZE_32M_MAP_2048_2048,  /**<  attention: don't support now ,just compatible for nodemcu;
                                           Flash size : 32Mbits. Map : 2048KBytes + 2048KBytes */
	FLASH_SIZE_64M_MAP_1024_1024,  /**<  Flash size : 64Mbits. Map : 1024KBytes + 1024KBytes */
	FLASH_SIZE_128M_MAP_1024_1024, /**<  Flash size : 128Mbits. Map : 1024KBytes + 1024KBytes */
};

enum flash_size_map system_get_flash_size_map(void);

void system_phy_set_max_tpw(uint8_t max_tpw);
void system_phy_set_tpw_via_vdd33(uint16_t vdd33);
void system_phy_set_rfoption(uint8_t option);
void system_phy_set_powerup_option(uint8_t option);

bool system_param_save_with_protect(uint16_t start_sec, void* param, uint16_t len);
bool system_param_load(uint16_t start_sec, uint16_t offset, void* param, uint16_t len);

void system_soft_wdt_stop(void);
void system_soft_wdt_restart(void);
void system_soft_wdt_feed(void);

void system_show_malloc(void);

typedef enum {
	SYSTEM_PARTITION_INVALID = 0,
	SYSTEM_PARTITION_BOOTLOADER,	   /* user can't modify this partition address, but can modify size */
	SYSTEM_PARTITION_OTA_1,			   /* user can't modify this partition address, but can modify size */
	SYSTEM_PARTITION_OTA_2,			   /* user can't modify this partition address, but can modify size */
	SYSTEM_PARTITION_RF_CAL,		   /* user must define this partition */
	SYSTEM_PARTITION_PHY_DATA,		   /* user must define this partition */
	SYSTEM_PARTITION_SYSTEM_PARAMETER, /* user must define this partition */
	SYSTEM_PARTITION_AT_PARAMETER,
	SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY,
	SYSTEM_PARTITION_SSL_CLIENT_CA,
	SYSTEM_PARTITION_SSL_SERVER_CERT_PRIVKEY,
	SYSTEM_PARTITION_SSL_SERVER_CA,
	SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY,
	SYSTEM_PARTITION_WPA2_ENTERPRISE_CA,

	SYSTEM_PARTITION_CUSTOMER_BEGIN = 100, /* user can define partition after here */
	SYSTEM_PARTITION_MAX
} partition_type_t;

typedef struct {
	partition_type_t type; /* the partition type */
	uint32_t addr;		   /* the partition address */
	uint32_t size;		   /* the partition size */
} partition_item_t;

/**
  * @brief     regist partition table information, user MUST call it in user_pre_init()
  *
  * @param     partition_table: the partition table
  * @param     partition_num:   the partition number in partition table
  * @param     map:             the flash map
  *
  * @return  true : succeed
  * @return false : fail
  */
bool system_partition_table_regist(const partition_item_t* partition_table, uint32_t partition_num, uint32_t map);

/**
  * @brief     get ota partition size
  *
  * @return    the size of ota partition
  */
uint32_t system_partition_get_ota_partition_size(void);

/**
  * @brief     get partition information
  *
  * @param     type:             the partition type
  * @param     partition_item:   the point to store partition information
  *
  * @return  true : succeed
  * @return false : fail
  */
bool system_partition_get_item(partition_type_t type, partition_item_t* partition_item);

#ifdef __cplusplus
}
#endif
