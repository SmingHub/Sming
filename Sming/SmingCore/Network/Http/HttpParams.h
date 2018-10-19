/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 * 	Class to manage HTTP URI query parameters
 *
 *  The HttpParams class was an empty HashMap class living in 'Structures.h'.
 *  It has been expanded to incorporate escaping and unescaping.
 *  Custom URL parsing code has been replaced with the yuarel library https://github.com/jacketizer/libyuarel
 *
 ****/

#ifndef _SMINGCORE_HTTP_HTTP_PARAMS_H_
#define _SMINGCORE_HTTP_HTTP_PARAMS_H_

#include "WString.h"
#include "WHashMap.h"
#include "Printable.h"

/** @brief
 *
 *  @todo values stored in escaped form, unescape return value and escape provided values.
 *  Revise HttpBodyParser.cpp as it will no longer do this job.
 *
 */
class HttpParams : public HashMap<String, String>, public Printable
{
public:
	/** @brief Called from URL class to process query section of a URI
	 *  @param query extracted from URI, excluding '&' prefix
	 *  @retval bool true on success, false if parsing failed
	 *  @note query string is modified by this call
	 */
	void parseQuery(char* query);

	/** @brief Return full escaped content for incorporation into a URI */
	String toString() const;

	/** @brief Obtain the parameter name and value at the given index, then remove it from the list
	 *  @param index
	 *  @param name String variable to store the returned parameter name
	 *  @param value the parameter value
	 *  @retval bool true on success, false if the parameter does not exist
	 */
	bool extract(unsigned index, String& name, String& value)
	{
		if(index >= count())
			return false;
		name = keyAt(index);
		value = valueAt(index);
		removeAt(index);
		return true;
	}

	// Printable
	virtual size_t printTo(Print& p) const;
};

#endif // _SMINGCORE_HTTP_HTTP_PARAMS_H_
