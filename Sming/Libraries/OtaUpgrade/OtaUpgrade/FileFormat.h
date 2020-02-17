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

#ifdef __cplusplus
extern "C" {
#endif

/** File header of an unencrypted OTA upgrade file.
 */
typedef struct {
	uint32_t magic; ///< File type identification, either #OTA_HEADER_MAGIC_SIGNED or #OTA_HEADER_MAGIC_NOT_SIGNED.
	uint32_t buildTimestampLow;  ///< File creation timestamp, Milliseconds since 1900/01/01 (lower 32 bits)
	uint32_t buildTimestampHigh; ///< File creation timestamp, Milliseconds since 1900/01/01 (lower 32 bits)
	uint8_t romCount;			 ///< Number of ROM images in this filem, each preceeded with an #OTA_RomHeader.
	uint8_t reserved[3];		 ///< Reserved bytes, must be zero for compatibility with future versions.
} OtaFileHeader;

/** Header of ROM image inside an OTA upgrade file.
 */
typedef struct {
	uint32_t address; ///< Flash memory destination offset for this ROM image.
	uint32_t size;	///< Size of ROM image content following this header, in bytes.
} OtaRomHeader;

/** Expected value for OTA_FileHeader::magic for digitally signed upgrad file. */
#define OTA_HEADER_MAGIC_SIGNED 0xf01af02a
/** Expected value for OTA_FileHeader::magic when signing is disabled. */
#define OTA_HEADER_MAGIC_NOT_SIGNED 0xf01af020

#ifdef __cplusplus
}
#endif
