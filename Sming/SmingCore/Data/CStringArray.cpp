/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CStringArray.cpp
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "CStringArray.h"

bool CStringArray::add(const char* str, unsigned length)
{
	if(length == 0)
		length = strlen(str);
	if(!reserve(len + length + 1))
		return false;
	if(len)
		buffer[len++] = '\0';			   // Separator between strings
	memcpy(buffer + len, str, length + 1); // Copy final nul terminator
	len += length;
	++stringCount;
	return true;
}

int CStringArray::indexOf(const char* str) const
{
	if(str == nullptr)
		return -1;

	unsigned index = 0;
	for(unsigned offset = 0; offset < len; ++index) {
		const char* s = buffer + offset;
		if(strcasecmp(str, s) == 0)
			return index;
		offset += strlen(s) + 1;
	}

	return -1;
}

const char* CStringArray::getValue(unsigned index) const
{
	if(index < count()) {
		for(unsigned offset = 0; offset <= length(); --index) {
			const char* s = buffer + offset;
			if(index == 0)
				return s;
			offset += strlen(s) + 1;
		}
	}

	return nullptr;
}

unsigned CStringArray::count() const
{
	if(stringCount == 0 && length() > 0) {
		//If array is created by assignment (e.g. CStringsArray csa = "Hello\0World";) then stringCount is not set so set it here
		for(unsigned offset = 0; offset <= length(); offset++) {
			if(buffer[offset] == '\0')
				++stringCount;
		}
	}
	return stringCount;
}
