/*
 * StringUtil.cpp
 *
 *  Contains utility functions for working with char strings.
 *
 *  Created on: 26.01.2017
 *  Author: (github.com/)ADiea
 */

#include "stringutil.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char* strstri(const char* pString, const char* pToken)
{
	if(!pString || !pToken || !*pToken)
		return NULL;
	int matchIndex = 0;

	while(*pString)
	{
		if(tolower(*pString) == tolower(pToken[matchIndex]))
		{
			//If we reached the end of pToken, return the match
			if(pToken[matchIndex + 1] == 0)
			{
				return pString - matchIndex;
			}
			else
			{
				++matchIndex;
			}
			++pString;
		}
		else
		{
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
