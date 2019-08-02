/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpBodyParser.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpCommon.h"
#include "HttpRequest.h"

/** @brief special length values passed to parse functions */
const int PARSE_DATASTART = -1; ///< Start of incoming data
const int PARSE_DATAEND = -2;   ///< End of incoming data

/**
 * @brief Body parser callback delegate
 * @param request
 * @param at
 * @param length Negative lengths have special meanings
 * @see `PARSE_DATASTART`
 * @see `PARSE_DATAEND`
 */
typedef Delegate<void(HttpRequest& request, const char* at, int length)> HttpBodyParserDelegate;

/**
 * @brief Maps body parsers to a specific content type
 */
typedef HashMap<String, HttpBodyParserDelegate> BodyParsers;

/**
 * @brief Parses application/x-www-form-urlencoded body data
 * @see `HttpBodyParserDelegate`
 */
void formUrlParser(HttpRequest& request, const char* at, int length);

/**
 * @brief Stores the complete body into memory
 * @see `HttpBodyParserDelegate`
 * @note The content later can be retrieved by calling request.getBody()
 */
void bodyToStringParser(HttpRequest& request, const char* at, int length);
