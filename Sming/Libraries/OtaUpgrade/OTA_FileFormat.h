/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OTA_FileFormat.h - Definitions for Upgrade file format
 *
 * Important: Definitions in this file must be kept in sync with otatool.py
 * See README.rst for further information.
 *
 ****/

#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint32_t magic;
	uint32_t buildTimestampLow;
	uint32_t buildTimestampHigh;
	uint8_t romCount;
	uint8_t reserved[3];
} OTA_FileHeader;

typedef struct __attribute__((packed)) {
	uint32_t address;
	uint32_t size;
} OTA_RomHeader;

#define OTA_HEADER_MAGIC_SIGNED 0xf01af02a
#define OTA_HEADER_MAGIC_NOT_SIGNED 0xf01af020
