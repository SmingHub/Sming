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

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_PARAMS_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_PARAMS_H_

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
	// Printable
	size_t printTo(Print& p) const override;
};

#endif /* _SMING_CORE_NETWORK_HTTP_HTTP_PARAMS_H_ */
