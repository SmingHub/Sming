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
#include <stringutil.h>

bool CStringArray::add(const char* str, int length)
{
	// Note: zero-length string is fine
	if(str == nullptr) {
		return true; // Nothing to append
	}

	int str_len = strlen(str);
	if(length < 0) {
		length = str_len;
	}
	auto len = this->length();

	// Ensure trailing NUL is included
	int addNul = (length == 0 || str[length - 1] != '\0') ? 1 : 0;
	if(!setLength(len + length + addNul)) {
		return false;
	}

	// Append the content
	auto buf = String::begin() + len;
	memcpy(buf, str, length);

	// Append NUL if one wasn't provided
	if(addNul) {
		buf += length;
		*buf = '\0';
	}

	// Were multiple strings added?
	if(length > str_len) {
		stringCount = 0; // Yes, recalculate count
	} else if(stringCount != 0) {
		++stringCount; // No, just one string
	}
	return true;
}

int CStringArray::indexOf(const char* str, bool ignoreCase) const
{
	for(auto it = begin(); it != end(); ++it) {
		if(ignoreCase) {
			if(it.equalsIgnoreCase(str)) {
				return it.index();
			}
		} else if(it.equals(str)) {
			return it.index();
		}
	}

	return -1;
}

const char* CStringArray::getValue(unsigned index) const
{
	if(index >= count()) {
		return nullptr;
	}

	for(auto it = begin(); it != end(); ++it) {
		if(it.index() == index) {
			return *it;
		}
	}

	return nullptr;
}

// Called when a string array is constructed or assigned
void CStringArray::init()
{
	// Ensure data has a trailing NUL
	auto len = length();
	if(len > 0 && String::begin()[len - 1] != '\0') {
		concat('\0');
	}
	// Reset string count so it's recalculated when requested
	stringCount = 0;
}

/*
 * When content is created by assignment (e.g. CStringArray csa = "Hello\0World";)
 * then stringCount is not set so set it here
 */
unsigned CStringArray::count() const
{
	auto len = length();
	if(stringCount == 0 && len > 0) {
		for(auto buf = c_str(); len > 0; --len, ++buf) {
			if(*buf == '\0') {
				++stringCount;
			}
		}
	}
	return stringCount;
}
