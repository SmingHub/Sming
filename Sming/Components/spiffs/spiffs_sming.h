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

#include <spiffs.h>
#include <Storage/Partition.h>

/**
 * @brief unmount SPIFFS filesystem
 * @deprecated use fileFreeFileSystem() instead
 * @note this will do nothing if the active filesystem is not SPIFFS
 */
void spiffs_unmount() SMING_DEPRECATED;

/** @brief Format and mount a SPIFFS filesystem
 *  @deprecated use fileSystemFormat() instead
 *  @note this will fail if the active filesystem is not SPIFFS
 */
bool spiffs_format() SMING_DEPRECATED;

/**
 * @brief Format and mount a SPIFFS volume using given partition
 * @param partition
 * @retval bool true on success
 */
bool spiffs_format(Storage::Partition& partition);
