/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HexString.h - Utility functions to deal with hex-encoded strings
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 */

#ifndef _SMING_CORE_DATA_HEX_STRING_H_
#define _SMING_CORE_DATA_HEX_STRING_H_

#include "WString.h"

/** @brief Convert data into a hexadecimal string representation
 *  @param data
 *  @param length
 *  @param separator optional character to put between hex-encoded bytes
 *  @retval String
 */
String makeHexString(const uint8_t* data, unsigned length, char separator = '\0');

#endif /* _SMING_CORE_DATA_HEX_STRING_H_ */
