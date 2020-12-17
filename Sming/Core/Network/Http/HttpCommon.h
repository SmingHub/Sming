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
 * @brief HTTP error codes
 */
enum class HttpError {
#define XX(n, s) n,
	HTTP_ERRNO_MAP(XX)
#undef XX
};

#define XX(n, s) constexpr HttpError HPE_##n = HttpError::n;
HTTP_ERRNO_MAP(XX)
#undef XX

/* Macro defined using C++ type. Internal http_parser code has own definition */
#define HTTP_PARSER_ERRNO(p) HttpError((p)->http_errno)

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

using HttpFiles = ObjectMap<String, ReadWriteStream>;

/**
 * @brief Return a descriptive string for the given error
 */
String toString(HttpError err);

/**
 * @brief Return a string name of the given error
 * @note This replaces the one in http_parser module which uses a load of RAM
 * @deprecated Use `toString(HttpError)`
 */
inline String httpGetErrorName(HttpError err) SMING_DEPRECATED;
inline String httpGetErrorName(HttpError err)
{
	return toString(err);
}

/**
 * @brief Return a descriptive string for the given error
 */
String httpGetErrorDescription(HttpError err);

/**
 * @brief Return a descriptive string for an HTTP status code
 */
String toString(HttpStatus code);

/**
 * @brief Return a descriptive string for an HTTP status code
 */
inline String httpGetStatusText(unsigned code)
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

/** @} */
