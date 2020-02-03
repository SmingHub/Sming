/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spiffs_sming.h
 *
 ****/
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "spiffs.h"
#include <stdbool.h>

#define LOG_PAGE_SIZE 256

/**
 * @brief Mount the SPIFFS volume using default configuration
 * @retval bool true on success
 *
 * Configuration is obtained `spiffs_get_storage_config()`.
 */
bool spiffs_mount();

/**
 * @brief Mount a SPIFFS volume using custom location and size
 * @param phys_addr The flash memory address (offset) for the volume
 * @param phys_size The volume size, in bytes
 * @retval bool true on success, false on failure
 * @note If the given flash memory range appears to be empty then it is
 * formatted, erasing any existing content.
 */
bool spiffs_mount_manual(uint32_t phys_addr, uint32_t phys_size);

/**
 * @brief Unmount a previously mounted volume
 */
void spiffs_unmount();

/**
 * @brief Format and mount a SPIFFS volume using default configuration
 * @retval bool true on success
 */
bool spiffs_format();

/**
 * @brief Format and mount a SPIFFS volume using custom location and size
 * @param phys_addr The flash memory address (offset) for the volume
 * @param phys_size The volume size, in bytes
 * @retval bool true on success
 */
bool spiffs_format_manual(uint32_t phys_addr, uint32_t phys_size);

/**
 * @brief Obtain the default SPIFFS configuration information
 * @retval spiffs_config
 * @note Only `phys_addr` and `phys_size` are used, all other parameters are overridden.
 */
spiffs_config spiffs_get_storage_config();

/**
 * @brief Global SPIFFS instance used by FileSystem API
 */
extern spiffs _filesystemStorageHandle;

#if defined(__cplusplus)
}
#endif
