/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpParams.h
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

#pragma once

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
	HttpParams() = default;

	HttpParams(const HttpParams& params)
	{
		*this = params;
	}

	HttpParams(String query)
	{
		parseQuery(query.begin());
	}

	/** @brief Called from URL class to process query section of a URI
	 *  @param query extracted from URI, with or without '?' prefix
	 *  @retval bool true on success, false if parsing failed
	 *  @note query string is modified by this call
	 */
	void parseQuery(char* query);

	/** @brief Return full escaped content for incorporation into a URI */
	String toString() const;

	operator String() const
	{
		return toString();
	}

	HttpParams& operator=(const HttpParams& params)
	{
		clear();
		setMultiple(params);
		return *this;
	}

	// Printable
	size_t printTo(Print& p) const override;

	/**
	 * @brief Printable output for debugging
	 * @param p
	 */
	void debugPrintTo(Print& p) const;
};
