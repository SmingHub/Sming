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
	auto buf = begin() + len;
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
	auto buflen = this->length();
	if(str == nullptr || buflen == 0) {
		return -1;
	}

	unsigned index = 0;
	for(unsigned offset = 0; offset < len; ++index) {
		const char* s = buffer + offset;
		if(ignoreCase) {
			if(strcasecmp(str, s) == 0) {
				return index;
			}
		} else if(strcmp(str, s) == 0) {
			return index;
		}
		offset += strlen(s) + 1;
	}

	return -1;
}

const char* CStringArray::getValue(unsigned index) const
{
	if(index >= count()) {
		return nullptr;
	}

	auto buf = c_str();
	auto len = length();
	for(unsigned offset = 0; offset < len; --index) {
		auto s = buf + offset;
		if(index == 0) {
			return s;
		}
		offset += strlen(s) + 1;
	}

	return nullptr;
}

// Called when a string array is constructed or assigned
void CStringArray::init()
{
	// Ensure data has a trailing NUL
	auto len = length();
	if(len > 0 && begin()[len - 1] != '\0') {
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
