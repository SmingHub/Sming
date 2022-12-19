#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sleep_type {
	NONE_SLEEP_T = 0,
	LIGHT_SLEEP_T,
	MODEM_SLEEP_T,
};

bool system_deep_sleep(uint32_t time_in_us);
bool system_deep_sleep_set_option(uint8_t option);

#ifdef __cplusplus
}
#endif
