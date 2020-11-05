/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpCommon.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#define ENABLE_HTTP_REQUEST_AUTH 1

#include "WString.h"
#include "../WebConstants.h"
#include "../Url.h"
#include "Data/Stream/ReadWriteStream.h"
#include "Data/ObjectMap.h"

#ifndef HTTP_MAX_HEADER_SIZE
#define HTTP_MAX_HEADER_SIZE (8 * 1024)
#endif

/* Number of maximum tcp connections to be kept in the pool */
#ifndef HTTP_REQUEST_POOL_SIZE
#define HTTP_REQUEST_POOL_SIZE 20
#endif

#include "http-parser/http_parser.h"

/**
 * @ingroup http
 * {
 */

/**
 * @brief Strongly-typed enum which shadows http_method from http_parser library
 */
enum class HttpMethod {
#define XX(num, name, string) name = num,
	HTTP_METHOD_MAP(XX)
#undef XX
};

#define XX(num, name, string) constexpr HttpMethod HTTP_##name = HttpMethod::name;
HTTP_METHOD_MAP(XX)
#undef XX

/**
 * @brief HTTP status code
 */
enum class HttpStatus {
#define XX(num, name, string) name = num,
	HTTP_STATUS_MAP(XX)
#undef XX
};

#define XX(num, name, string) constexpr HttpStatus HTTP_STATUS_##name = HttpStatus::name;
HTTP_STATUS_MAP(XX)
#undef XX

/**
 * @brief Identifies current state for an HTTP connection
 */
enum HttpConnectionState {
	eHCS_Ready = 0,
	eHCS_StartSending,
	eHCS_SendingHeaders,
	eHCS_StartBody,
	eHCS_SendingBody,
	eHCS_Sent,
	eHCS_WaitResponse
};

typedef ObjectMap<String, ReadWriteStream> HttpFiles;

/**
 * @brief Return a string name of the given error
 * @note This replaces the one in http_parser module which uses a load of RAM
 */
String httpGetErrorName(enum http_errno err);

inline String toString(enum http_errno err)
{
	return httpGetErrorName(err);
}

/**
 * @brief Return a descriptive string for the given error
 */
String httpGetErrorDescription(enum http_errno err);

/**
 * @brief Return a descriptive string for an HTTP status code
 */
String toString(HttpStatus code);

/**
 * @brief Return a descriptive string for an HTTP status code
 */
static inline String httpGetStatusText(unsigned code)
{
	return toString(HttpStatus(code));
}

/**
 * @brief Return text for an HTTP method
 */
inline String toString(HttpMethod method)
{
	return http_method_str(http_method(method));
}
