/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#ifndef _SMING_CORE_DATA_STRING_ARRAY_H_
#define _SMING_CORE_DATA_STRING_ARRAY_H_

#include "WString.h"

/** @brief Class to manage a double null-terminated list of strings, such as "one\0two\0three"
 * 	@note
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
 */
class CStringArray : protected String
{
public:
	// Inherit all constructors
	using String::String;

	/** @brief append a new string to the end of the array
	 *  @param str
	 *  @retval bool false on memory allocation error
	 */
	bool add(const char* str, unsigned length = 0);

	bool add(const String& str)
	{
		return add(str.c_str(), str.length());
	}

	/** @brief Find the given string and return its index
	 * 	@param str String to find
	 * 	@retval int index of given string, -1 if not found
	 * 	@note comparison is not case-sensitive
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

	/** @brief Get string at the given position
	 * 	@param index 0-based index of string to obtain
	 * 	@retval const char* nullptr if index is not valid
	 */
	const char* getValue(unsigned index) const;

	const char* operator[](unsigned index) const
	{
		return getValue(index);
	}

	/** @brief Empty the array
	 */
	void clear()
	{
		invalidate();
		stringCount = 0;
	}

	unsigned count() const
	{
		return stringCount;
	}

private:
	unsigned stringCount = 0;
};

#endif // _SMING_CORE_DATA_STRING_ARRAY_H_
