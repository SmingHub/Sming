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

#ifndef SPI_OVERLAP_APP_H
#define SPI_OVERLAP_APP_H

#include "ets_sys.h"
#include "spi_flash.h"
#define HSPI_OVERLAP
//#define NO_HSPI_DEVICE
#define HOST_INF_SEL 0x3ff00028 
#define FUNC_SPI_CS2 1
#define FUNC_SPI_CS1 1
#define reg_cspi_overlap  (BIT7)

#define SPI_FLASH_BYTES_LEN                24
#define IODATA_START_ADDR                 BIT0
#define SPI_BUFF_BYTE_NUM                    32

#define PERIPHS_IO_MUX_BACKUP		0
#define SPI_USER_BACKUP  	1
#define SPI_CTRL_BACKUP  	2 
#define SPI_CLOCK_BACKUP 	3
#define SPI_USER1_BACKUP	4
#define SPI_USER2_BACKUP	5
#define SPI_CMD_BACKUP		6
#define SPI_PIN_BACKUP		7
#define SPI_SLAVE_BACKUP	8

#define HSPI_CS_DEV			0
#define SPI_CS1_DEV			1
#define SPI_CS2_DEV			2
#define SPI_CS0_FLASH		3
#define HSPI_IDLE			4

struct hspi_device_config{
	uint8 active:1;
	uint8 clk_polar:1;
	uint8 res:1;
	uint8 clk_div:5;
};

struct hspi_device_register{
	uint32 hspi_flash_reg_backup[9];
	uint32 hspi_dev_reg_backup[9];
	struct hspi_device_config hspi_dev_conf[4];
	uint8 selected_dev_num:3;
	uint8 spi_io_80m:1;
	uint8 hspi_reg_backup_flag:1;
	uint8 res:3;
};

void hspi_overlap_init(void);
void hspi_overlap_deinit(void);
void spi_reg_recover(uint8 spi_no,uint32* backup_mem);
void spi_reg_backup(uint8 spi_no,uint32* backup_mem);

void hspi_master_dev_init(uint8 dev_no,uint8 clk_polar,uint8 clk_div);
void hspi_dev_sel(uint8 dev_no);

void hspi_overlap_flash_init(void);
SpiFlashOpResult hspi_overlap_read_flash_data(SpiFlashChip * spi, uint32 flash_addr, uint32 * addr_dest, uint32 byte_length);

#endif
