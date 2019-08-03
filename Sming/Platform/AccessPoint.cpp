/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPoint.cpp
 *
 ****/

#include "AccessPoint.h"
#include <Data/HexString.h>

String AccessPointClass::getMAC(char sep) const
{
	uint8 hwaddr[6];
	if(getMacAddr(hwaddr)) {
		return makeHexString(hwaddr, sizeof(hwaddr), sep);
	} else {
		return nullptr;
	}
}
