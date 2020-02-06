/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpCommon.cpp
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 * 	httpGetErrorName(), httpGetErrorDescription() and httpGetStatusText() functions added
 *
 ****/

#include "HttpCommon.h"
#include <FlashString/Vector.hpp>
#include <FlashString/Map.hpp>

// Define flash strings and lookup table for HTTP error names
#define XX(name, string) DEFINE_FSTR_LOCAL(hpename_##name, "HPE_" #name);
HTTP_ERRNO_MAP(XX)
#undef XX

#define XX(name, string) &hpename_##name,
DEFINE_FSTR_VECTOR_LOCAL(hpeNames, FlashString, HTTP_ERRNO_MAP(XX));
#undef XX

String httpGetErrorName(enum http_errno err)
{
	String s = hpeNames[err];
	return s ?: F("HPE_#") + String(err);
}

// Define flash strings and lookup table for HTTP error descriptions
#define XX(name, string) DEFINE_FSTR_LOCAL(hpedesc_##name, string);
HTTP_ERRNO_MAP(XX)
#undef XX

#define XX(_n, _s) &hpedesc_##_n,
DEFINE_FSTR_VECTOR_LOCAL(hpeDescriptions, FlashString, HTTP_ERRNO_MAP(XX));
#undef XX

String httpGetErrorDescription(enum http_errno err)
{
	String s = hpeDescriptions[err];
	return s ?: F("HPE_#") + String(err);
}

// Define flash strings and Map for HTTP status codes
#define XX(num, name, string) DEFINE_FSTR_LOCAL(hpsText_##name, #string);
HTTP_STATUS_MAP(XX)
#undef XX

#define XX(num, name, string) {HTTP_STATUS_##name, &hpsText_##name},
DEFINE_FSTR_MAP_LOCAL(httpStatusMap, enum http_status, FlashString, HTTP_STATUS_MAP(XX));
#undef XX

String httpGetStatusText(enum http_status code)
{
	auto s = String(httpStatusMap[code]);
	return s ?: F("<unknown_") + String(code) + '>';
}
