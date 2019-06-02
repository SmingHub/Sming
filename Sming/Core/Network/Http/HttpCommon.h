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

#ifndef HTTP_MAX_HEADER_SIZE
#define HTTP_MAX_HEADER_SIZE (8 * 1024)
#endif

/* Number of maximum tcp connections to be kept in the pool */
#ifndef HTTP_REQUEST_POOL_SIZE
#define HTTP_REQUEST_POOL_SIZE 20
#endif

#include "http-parser/http_parser.h"

typedef enum http_method HttpMethod;

enum HttpConnectionState {
	eHCS_Ready = 0,
	eHCS_StartSending,
	eHCS_SendingHeaders,
	eHCS_StartBody,
	eHCS_SendingBody,
	eHCS_Sent
};

/**
 * @brief Return a string name of the given error
 * @param err
 * @retval String
 * @note This replaces the one in http_parser module which uses a load of RAM
 */
String httpGetErrorName(enum http_errno err);

/**
 * @brief Return a descriptive string for the given error
 * @param err
 * @retval String
 */
String httpGetErrorDescription(enum http_errno err);

/**
 * @brief Return a descriptive string for an HTTP status code
 * @param code
 * @retval String
 */
String httpGetStatusText(enum http_status code);

/**
 * @brief Return a descriptive string for an HTTP status code
 * @param code
 * @retval String
 */
static inline String httpGetStatusText(unsigned code)
{
	return httpGetStatusText((enum http_status)code);
}
