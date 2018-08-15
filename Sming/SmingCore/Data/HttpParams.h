/*
 * HttpParams.h
 *
 *  Created on: 1 Aug 2018
 *      Author: mikee47
 *
 * Class to manage HTTP URI parameters
 */

#ifndef _SMINGCORE_DATA_HTTPPARAMS_H_
#define _SMINGCORE_DATA_HTTPPARAMS_H_

#include "WString.h"
#include "WHashMap.h"


/* @todo values stored in escaped form, unescape return value and
 * escape provided values. Revise HttpBodyParser.cpp as it will no
 * longer do this job.
 *
 */
class HttpParams: public HashMap<String, String>
{
public:
	HttpParams()
	{
		// By default we get a null string, so if (str) evaluates false
		setNullValue(nullptr);
	}

	/** @brief extract the parameter name and value at the given index.
	 *  @param index
	 *  @param name OUT the parameter name
	 *  @retval String the parameter value
	 *  @note returns a null String if index is not valid
	 */
	String extract(unsigned index, String& name)
	{
		if (index >= count())
			return nullptr;
		name = keyAt(index);
		String value = valueAt(index);
		removeAt(index);
		return value;
	}

	using HashMap::operator[];

	const String& operator[](const String& name) const
	{
		int i = indexOf(name);
		return (i >= 0) ? valueAt(i) : String::nullstr;
	}

};



#endif // _SMINGCORE_DATA_HTTPPARAMS_H_
