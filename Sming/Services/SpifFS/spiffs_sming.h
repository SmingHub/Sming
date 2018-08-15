
#ifndef SPIFFS_SMING_H_
#define SPIFFS_SMING_H_
#if defined(__cplusplus)
extern "C" {
#endif

#include "spiffs.h"

#define SPIFFS_LOG_PAGE_SIZE       256

bool spiffs_mount();
bool spiffs_mount_manual(u32_t phys_addr, u32_t phys_size);
void spiffs_unmount();
bool spiffs_format();
bool spiffs_format_internal(spiffs_config *cfg);
bool spiffs_format_manual(u32_t phys_addr, u32_t phys_size);
spiffs_config spiffs_get_storage_config();

extern spiffs _filesystemStorageHandle;

#if defined(__cplusplus)
}
#endif
#endif /* SPIFFS_SMING_H_ */
