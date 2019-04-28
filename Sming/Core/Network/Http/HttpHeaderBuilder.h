/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpHeaderBuilder.h
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_HEADER_BUILDER_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_HEADER_BUILDER_H_

#include "HttpHeaders.h"

/** @brief Re-assembles headers from fragments via onHeaderField / onHeaderValue callbacks */
class HttpHeaderBuilder
{
public:
	int onHeaderField(const char* at, size_t length)
	{
		if(lastWasValue) {
			// we are starting to process new header - setLength keeps allocated memory
			lastData.setLength(0);
			lastWasValue = false;
		}
		lastData.concat(at, length);

		return 0;
	}

	int onHeaderValue(HttpHeaders& headers, const char* at, size_t length)
	{
		if(!lastWasValue) {
			currentField = lastData;
			headers[currentField] = nullptr;
			lastWasValue = true;
		}
		headers[currentField].concat(at, length);
		return 0;
	}

	void reset()
	{
		lastWasValue = true;
		lastData = nullptr;
		currentField = nullptr;
	}

private:
	bool lastWasValue = true; ///< Indicates whether last callback was Field or Value
	String lastData;		  ///< Content of field or value, may be constructed over several callbacks
	String currentField;	  ///< Header field name
};

#endif /* _SMING_CORE_NETWORK_HTTP_HTTP_HEADER_BUILDER_H_ */
