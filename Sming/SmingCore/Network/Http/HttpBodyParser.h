/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpBodyParser
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_HTTP_BODY_PARSER_H_
#define _SMING_CORE_HTTP_BODY_PARSER_H_

#include "HttpCommon.h"
#include "HttpRequest.h"

/** @brief special length values passed to parse functions */
#define PARSE_DATASTART -1
#define PARSE_DATAEND -2

typedef Delegate<void(HttpRequest&, const char* at, int length)> HttpBodyParserDelegate;
typedef HashMap<String, HttpBodyParserDelegate> BodyParsers;

typedef struct {
	char searchChar = '=';
	String postName;
} FormUrlParserState;

/**
 * @brief Parses application/x-www-form-urlencoded body data
 * @param HttpRequest&
 * @param const *char
 * @param int length Negative lengths are used to specify special cases
 * 				-1 - start of incoming data
 * 				-2 - end of incoming data
 */
void formUrlParser(HttpRequest& request, const char* at, int length);

/**
 * @brief Stores the complete body into memory.
 *        The content later can be retrieved by calling request.getBody()
 * @param HttpRequest&
 * @param const *char
 * @param int length Negative lengths are used to specify special cases
 * 				-1 - start of incoming data
 * 				-2 - end of incoming data
 */
void bodyToStringParser(HttpRequest& request, const char* at, int length);

#endif /* _SMING_CORE_HTTP_BODY_PARSER_H_ */
