/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StringUtil.cpp
 *
 *  Contains utility functions for working with char strings.
 *
 *  Created on: 26.01.2017
 *  Author: (github.com/)ADiea
 *
 */

#include "stringutil.h"
#include <cstdlib>
#include <cstring>
#include <cctype>

const char* strstri(const char* pString, const char* pToken)
{
	if(!pString || !pToken || !*pToken)
		return NULL;
	int matchIndex = 0;

	while(*pString) {
		if(tolower(*pString) == tolower(pToken[matchIndex])) {
			//If we reached the end of pToken, return the match
			if(pToken[matchIndex + 1] == 0) {
				return pString - matchIndex;
			} else {
				++matchIndex;
			}
			++pString;
		} else {
			//If we were in the middle of a matching process,
			// recheck current pString character with
			// the first pToken character else increase pString
			if(matchIndex)
				matchIndex = 0;
			else
				++pString;
		}
	}
	return NULL;
}

int memicmp(const void* buf1, const void* buf2, size_t len)
{
	auto p1 = static_cast<const unsigned char*>(buf1);
	auto p2 = static_cast<const unsigned char*>(buf2);

	if(p1 == p2) {
		return 0;
	}

	int result = 0;
	while(len-- && (result = tolower(*p1++) - tolower(*p2++)) == 0) {
	}

	return result;
}

char hexchar(unsigned char c)
{
	if(c < 10) {
		return (char)('0' + c);
	}
	if(c <= 15) {
		return (char)('a' + c - 10);
	}
	return '\0';
}

signed char unhex(char c)
{
	if(c >= '0' && c <= '9') {
		return (char)(c - '0');
	}
	if(c >= 'a' && c <= 'f') {
		return (char)(10 + c - 'a');
	}
	if(c >= 'A' && c <= 'F') {
		return (char)(10 + c - 'A');
	}
	return -1;
}
