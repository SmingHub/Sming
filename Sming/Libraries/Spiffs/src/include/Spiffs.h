/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Spiffs.h
 *
 ****/
#pragma once

#include <IFS/FileSystem.h>

namespace IFS
{
/**
 * @brief Create a SPIFFS filesystem
 * @param partition
 * @retval FileSystem* constructed filesystem object
 */
FileSystem* createSpiffsFilesystem(Storage::Partition partition);

} // namespace IFS

/**
 * @brief Mount the first available SPIFFS volume
 * @retval bool true on success
 */
bool spiffs_mount();

/**
 * @brief Mount SPIFFS volume from a specific partition
 */
bool spiffs_mount(Storage::Partition partition);

/**
 * @brief Mount the first available FWFS and SPIFFS partitions as a hybrid filesystem
 * @retval bool true on success
 */
bool hyfs_mount();

/**
 * @brief Mount the given FWFS and SPIFFS partitions as a hybrid filesystem
 * @retval bool true on success
 */
bool hyfs_mount(Storage::Partition fwfsPartition, Storage::Partition spiffsPartition);

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
