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

#define LOG_PAGE_SIZE       256

void spiffs_mount();
void spiffs_mount_manual(u32_t phys_addr, u32_t phys_size);
void spiffs_unmount();
bool spiffs_format();
bool spiffs_format_internal(spiffs_config *cfg);
bool spiffs_format_manual(u32_t phys_addr, u32_t phys_size);
spiffs_config spiffs_get_storage_config();

extern spiffs _filesystemStorageHandle;

#if defined(__cplusplus)
}
#endif
