/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 *	Class to manage a double null-terminated list of strings, such as "one\0two\0three"
 *
 *	Comparison with Vector<String>
 *
 *	Advantages:
 *		More memory efficient. Uses only a single heap allocation
 *		Useful for simple lookups, e.g. mapping enumerated values to strings
 *
 * 	Disadvantages:
 *
 * 		Slower. Items must be iterated using multiple strlen() calls
 * 		Ordering not supported
 * 		Insertions / deletions not supported (yet)
 *
 ****/

#ifndef WIRING_SZSTRING_H_
#define WIRING_SZSTRING_H_

#include "WString.h"

class SzString : public String
{
public:
	bool append(const char* str)
	{
		if(length()) {
			concat('\0');
		}
		return concat(str);
	}

	bool append(const String& str)
	{
		return append(str.c_str());
	}

	/*
	 * Find the given string and return its index.
	 *
	 * @param str String to find
	 * @retval int index of given string, -1 if not found
	 * @note comparison is not case-sensitive
	 */
	int indexOf(const char* str) const;

	int indexOf(const String& str) const
	{
		return indexOf(str.c_str());
	}

	bool contains(const char* str) const
	{
		return indexOf(str) >= 0;
	}

	bool contains(const String& str) const
	{
		return indexOf(str) >= 0;
	}

	/*
	 * Get string at a specific index.
	 *
	 * @param idx string index to retrieve
	 * @returns nullptr if index is not valid
	 */
	const char* getText(uint8_t value) const;

	void clear()
	{
		invalidate();
	}
};

#endif /* WIRING_SZSTRING_H_ */
