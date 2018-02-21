#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

	// UART config
	#define SERIAL_BAUD_RATE COM_SPEED_SERIAL

	// ESP SDK config
	#define LWIP_OPEN_SRC
	#define USE_US_TIMER

	// Default types
	#define __CORRECT_ISO_CPP_STDLIB_H_PROTO
	#include <limits.h>
	#include <stdint.h>

	// Remove buggy espconn
	#define _NO_ESPCON_

#ifdef SDK_INTERNAL
	// ESP SDK  2.1 or later provide proper c_types.h
	#include "c_types.h"
#else
	// Older SDKs, have wrong or incompatible c_types type definitions
	#define _C_TYPES_H_
	#include <espinc/c_types_compatible.h>
#endif /* SDK_INTERNAL */

	// System API declarations
	#include <esp_systemapi.h>

	// C++ Support
	#include <esp_cplusplus.h>
	// Extended string conversion for compatibility
	#include <stringconversion.h>
	// Network base API
	#include <espinc/lwip_includes.h>

	// Beta boards
	#define BOARD_ESP01

#ifdef __cplusplus
}
#endif

#endif
