/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpServerResource
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTP_COMMON_H_
#define _SMING_CORE_HTTP_COMMON_H_

#define ENABLE_HTTP_REQUEST_AUTH 1

#include "WString.h"
#include "../WebConstants.h"
#include "../URL.h"

#ifndef HTTP_MAX_HEADER_SIZE
#define HTTP_MAX_HEADER_SIZE (8 * 1024)
#endif

/* Number of maximum tcp connections to be kept in the pool */
#ifndef HTTP_REQUEST_POOL_SIZE
#define HTTP_REQUEST_POOL_SIZE 20
#endif

/* This stuff is all related to the HTTP header parser.
 *
 * @todo Create a wrapper class for the parser to eliminate duplication of code and
 * simplify further the HttpConnection and HttpServerConnection classes.
 *
 * 	- ressembling header/value into complete strings
 * 	- dealing with callbacks and associated protocols/semantics; we can use higher-level ones
 *
 * This could potentially be handled by using HttpConnection as the base class for both
 * HttpServerConnection and HttpClientConnection classes.
 *
 *
 */

#include "../http-parser/http_parser.h"

/** @brief special values returned by the on_headers_complete callback
 *  @note from http_parser.h line 70 -
 *
 * 		Callbacks should return non-zero to indicate an error. The parser will
 * 		then halt execution.
 *
 * 		The one exception is on_headers_complete. In a HTTP_RESPONSE parser
 * 		returning '1' from on_headers_complete will tell the parser that it
 * 		should not expect a body. This is used when receiving a response to a
 * 		HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * 		chunked' headers that indicate the presence of a body.
 *
 * 		Returning `2` from on_headers_complete will tell parser that it should not
 * 		expect neither a body nor any further responses on this connection. This is
 * 		useful for handling responses to a CONNECT request which may not contain
 * 		`Upgrade` or `Connection: upgrade` headers.
 */
#define HPE_HEADERS_NO_BODY 1
#define HPE_HEADERS_NO_FURTHER_RESPONSES 2

typedef enum http_method HttpMethod;

/**
 * 13/8/2018 (mikee47)
 *
 * @brief HTTP parser uses static callback functions. These macros are used to simplify code
 * to map these onto class methods by defining static handlers. The methods use the same
 * name as the parser callback functions. The function table is stored in flash
 * memory to save RAM; as it is word-aligned it can be accessed directly.
 */

// Callback with 0 arguments
#define HTTP_PARSER_METHOD_0(_cls, _method)                                                                            \
public:                                                                                                                \
	static int static_##_method(http_parser* parser)                                                                   \
	{                                                                                                                  \
		auto connection = (_cls*)parser->data;                                                                         \
		return connection ? connection->_method() : -1;                                                                \
	}                                                                                                                  \
                                                                                                                       \
private:                                                                                                               \
	int _method();

// Callback with 2 arguments
#define HTTP_PARSER_METHOD_2(_cls, _method)                                                                            \
public:                                                                                                                \
	static int static_##_method(http_parser* parser, const char* at, size_t length)                                    \
	{                                                                                                                  \
		auto connection = (_cls*)parser->data;                                                                         \
		return connection ? connection->_method(at, length) : -1;                                                      \
	}                                                                                                                  \
                                                                                                                       \
private:                                                                                                               \
	int _method(const char* at, size_t length);

// Unimplemented method - use as a placeholder
#define HTTP_PARSER_METHOD_N(_cls, _method)

// The method name for emitting parser settings table
#define HTTP_PARSER_CALLBACK(_cls, _method)                                                                            \
	_method:                                                                                                           \
	_cls::static_##_method

// Unused callback in table
#define HTTP_PARSER_CBNULL(_cls, _method)                                                                              \
	_method:                                                                                                           \
	nullptr

/*
 * See onReadyToSendData method of HttpConnection/HttpServerConnection
 */
enum HttpConnectionState {
	eHCS_Ready = 0,
	eHCS_StartSending,
	eHCS_SendingHeaders,
	eHCS_StartBody,
	eHCS_SendingBody,
	eHCS_Sent
};

String httpGetErrnoName(enum http_errno err);

String httpGetStatusText(enum http_status code);

static inline String httpGetStatusText(unsigned code)
{
	return httpGetStatusText((enum http_status)code);
}

#endif /* _SMING_CORE_HTTP_COMMON_H_ */
