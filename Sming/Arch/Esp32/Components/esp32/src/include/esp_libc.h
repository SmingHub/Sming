#pragma once

#include <rom/ets_sys.h>

#ifdef __cplusplus
extern "C" {
#endif

inline void system_set_os_print(bool onoff)
{
	//
}

#define ets_memcpy(dest, src, n) memcpy(dest, src, n)

#ifdef __cplusplus
}
#endif
