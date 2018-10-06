/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "SzString.h"

int SzString::indexOf(const char* str) const
{
	if(str == nullptr)
		return -1;

	int index = 0;
	for(unsigned offset = 0; offset < length(); ++index) {
		const char* p = c_str() + offset;
		if(strcasecmp(str, p) == 0)
			return index;
		offset += strlen(p) + 1;
	}

	return -1;
}

const char* SzString::getText(uint8_t value) const
{
	for(unsigned offset = 0; offset < length(); --value) {
		const char* p = c_str() + offset;
		if(value == 0)
			return *p ? p : nullptr;
		offset += strlen(p) + 1;
	}

	return nullptr;
}
