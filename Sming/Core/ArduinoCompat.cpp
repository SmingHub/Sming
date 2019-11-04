/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ArduinoCompact.cpp - Arduino Compatibility Layer
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "ArduinoCompat.h"

/**
 * @brief This method just feeds the software watchdog. It does not really switch the programming context as yield in Arduino does.
 */
void yield()
{
	system_soft_wdt_feed();
}
