#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t os_random(void);
int os_get_random(uint8_t* buf, size_t len);

#ifdef __cplusplus
}
#endif
