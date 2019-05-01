/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * esp_cplusplus.h
 *
 ****/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	#include "esp_systemapi.h"
#ifdef __cplusplus
}
#endif

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

void cpp_core_initialize();
