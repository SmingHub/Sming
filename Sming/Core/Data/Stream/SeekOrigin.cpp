/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SeekOrigin.cpp
 *
 ****/

#include "SeekOrigin.h"
#include <FlashString/Vector.hpp>

String toString(SeekOrigin origin)
{
	switch(origin) {
	case SeekOrigin::Start:
		return F("Start");
	case SeekOrigin::Current:
		return F("Current");
	case SeekOrigin::End:
		return F("End");
	default:
		return nullptr;
	}
}
