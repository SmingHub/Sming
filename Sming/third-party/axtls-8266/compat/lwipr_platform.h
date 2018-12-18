/*
 * lwipr_platform.h
 *
 *  Created on: Feb 8, 2016
 *      Author: slavey
 *
 */

#ifndef AXTLS_8266_COMPAT_LWIPR_PLATFORM_H_
#define AXTLS_8266_COMPAT_LWIPR_PLATFORM_H_

/* Add here all platform specific things */

#include <user_config.h>
//#define WATCHDOG_RESET() WDT.alive()


/*
 * Define the AXL_DEBUG_PRINT function to add debug functionality
 */
#ifndef AXL_DEBUG
	#define AXL_DEBUG_PRINT(...)
#else
	#define AXL_DEBUG_PRINT(...)  debugf(__VA_ARGS__)
#endif


#endif /* AXTLS_8266_COMPAT_LWIPR_PLATFORM_H_ */
