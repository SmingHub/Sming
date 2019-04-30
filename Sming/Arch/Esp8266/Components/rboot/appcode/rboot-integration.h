/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * rboot-integration.h
 *
 ****/

#pragma once

// prevent sming user_config.h being included
#define __USER_CONFIG_H__

// mark functions that need to be in iram
#define IRAM_ATTR __attribute__((section(".iram.text")))

// missing prototypes for sdk functions
#include <esp_systemapi.h>
