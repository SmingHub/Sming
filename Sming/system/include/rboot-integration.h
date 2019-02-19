/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * rboot-integration.h
 *
 ****/

#ifndef _SYSTEM_INCLUDE_RBOOT_INTEGRATION_H_
#define _SYSTEM_INCLUDE_RBOOT_INTEGRATION_H_

// prevent sming user_config.h being included
#define __USER_CONFIG_H__

// mark functions that need to be in iram
#define IRAM_ATTR __attribute__((section(".iram.text")))

// missing prototypes for sdk functions
#include <esp_systemapi.h>

#endif /* _SYSTEM_INCLUDE_RBOOT_INTEGRATION_H_ */
