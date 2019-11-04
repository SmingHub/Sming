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

String AccessPointClass::getMAC(char sep) const
{
	auto mac = getMacAddress();
	return mac ? mac.toString(sep) : nullptr;
}
