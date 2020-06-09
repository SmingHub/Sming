// Copyright 2018-2025 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file i2c_apll.h
 * @brief Register definitions for digital PLL (BBPLL)
 *
 * This file lists register fields of BBPLL, located on an internal configuration bus.
 */

#pragma once

#include <stdint.h>

#define i2c_bbpll 0x67
#define i2c_bbpll_en_audio_clock_out 4
#define i2c_bbpll_en_audio_clock_out_msb 7
#define i2c_bbpll_en_audio_clock_out_lsb 7
#define i2c_bbpll_hostid 4

#ifdef __cplusplus
extern "C" {
#endif

/* ROM functions which read/write internal control bus */
uint8_t rom_i2c_readReg(uint8_t block, uint8_t host_id, uint8_t reg_add);
uint8_t rom_i2c_readReg_Mask(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t msb, uint8_t lsb);
void rom_i2c_writeReg(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t data);
void rom_i2c_writeReg_Mask(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t msb, uint8_t lsb, uint8_t data);

#ifdef __cplusplus
}
#endif

#define i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)                                                   \
	rom_i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)
#define i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb) rom_i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb)
#define i2c_writeReg_Mask_def(block, reg_add, indata)                                                                  \
	i2c_writeReg_Mask(block, block##_hostid, reg_add, reg_add##_msb, reg_add##_lsb, indata)
#define i2c_readReg_Mask_def(block, reg_add)                                                                           \
	i2c_readReg_Mask(block, block##_hostid, reg_add, reg_add##_msb, reg_add##_lsb)
