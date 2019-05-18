#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t os_random(void);
int os_get_random(uint8_t* buf, size_t len);

void ets_install_putc1(void (*p)(char c));
void system_set_os_print(bool onoff);

#define ets_memcpy(dest, src, n) memcpy(dest, src, n)

#ifdef __cplusplus
}
#endif
