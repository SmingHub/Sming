/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpCommon.cpp
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

String toString(HttpError err)
{
	String s = hpeNames[unsigned(err)];
	return s ?: F("HPE_#") + String(unsigned(err));
}

// Define flash strings and lookup table for HTTP error descriptions
#define XX(name, string) DEFINE_FSTR_LOCAL(hpedesc_##name, string);
HTTP_ERRNO_MAP(XX)
#undef XX

#define XX(_n, _s) &hpedesc_##_n,
DEFINE_FSTR_VECTOR_LOCAL(hpeDescriptions, FlashString, HTTP_ERRNO_MAP(XX));
#undef XX

String httpGetErrorDescription(HttpError err)
{
	String s = hpeDescriptions[unsigned(err)];
	return s ?: F("HPE_#") + String(unsigned(err));
}

// Define flash strings and Map for HTTP status codes
#define XX(num, name, string) DEFINE_FSTR_LOCAL(hpsText_##name, #string);
HTTP_STATUS_MAP(XX)
#undef XX

#define XX(num, name, string) {HTTP_STATUS_##name, &hpsText_##name},
DEFINE_FSTR_MAP_LOCAL(httpStatusMap, HttpStatus, FlashString, HTTP_STATUS_MAP(XX));
#undef XX

String toString(HttpStatus code)
{
	auto s = String(httpStatusMap[code]);
	return s ?: F("<unknown_") + String(unsigned(code)) + '>';
}
