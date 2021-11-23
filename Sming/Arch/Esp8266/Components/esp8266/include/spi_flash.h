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

#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SPI_FLASH_RESULT_OK,
	SPI_FLASH_RESULT_ERR,
	SPI_FLASH_RESULT_TIMEOUT,
} SpiFlashOpResult;

typedef struct {
	uint32_t deviceId;
	uint32_t chip_size; // chip size in byte
	uint32_t block_size;
	uint32_t sector_size;
	uint32_t page_size;
	uint32_t status_mask;
} SpiFlashChip;

#define SPI_FLASH_SEC_SIZE 4096

uint32_t spi_flash_get_id(void);
SpiFlashOpResult spi_flash_erase_sector(uint16_t sec);
SpiFlashOpResult spi_flash_write(uint32_t des_addr, uint32_t* src_addr, uint32_t size);
SpiFlashOpResult spi_flash_read(uint32_t src_addr, uint32_t* des_addr, uint32_t size);

typedef SpiFlashOpResult (*user_spi_flash_read)(SpiFlashChip* spi, uint32_t src_addr, uint32_t* des_addr,
												uint32_t size);

void spi_flash_set_read_func(user_spi_flash_read read);

bool spi_flash_erase_protect_enable(void);
bool spi_flash_erase_protect_disable(void);

#ifdef __cplusplus
}
#endif
