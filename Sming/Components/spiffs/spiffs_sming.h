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

#include "spiffs.h"
#include <Storage/Partition.h>

/**
 * @brief Mount the SPIFFS volume using default configuration
 * @retval bool true on success
 *
 * Configuration is obtained `spiffs_get_storage_config()`.
 */
bool spiffs_mount();

/**
 * @brief Mount SPIFFS volume from a specific partition
 */
bool spiffs_mount(Storage::Partition partition);

/**
 * @brief unmount SPIFFS filesystem
 * @deprecated use fileFreeFileSystem() instead
 * @note this will do nothing if the active filesystem is not SPIFFS
 */
void spiffs_unmount();

/** @brief Format and mount a SPIFFS filesystem
 *  @deprecated use fileSystemFormat() instead
 *  @note this will fail if the active filesystem is not SPIFFS
 */
bool spiffs_format();

/**
 * @brief Format and mount a SPIFFS volume using given partition
 * @param partition
 * @retval bool true on success
 */
bool spiffs_format(Storage::Partition& partition);

/**
 * @brief Global SPIFFS instance used by FileSystem API
 */
extern spiffs _filesystemStorageHandle;
