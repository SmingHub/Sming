/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MultipartParser.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Http/HttpCommon.h>
#include <Network/Http/HttpRequest.h>

#include "multipart-parser/multipart_parser.h"

class MultipartParser
{
public:
	explicit MultipartParser(HttpRequest* request);
	~MultipartParser();

	size_t execute(const char* at, size_t length);

	static int readHeaderName(multipart_parser_t* p, const char* at, size_t length);
	static int readHeaderValue(multipart_parser_t* p, const char* at, size_t length);
	static int partBegin(multipart_parser_t* p);
	static int partData(multipart_parser_t* p, const char* at, size_t length);
	static int partEnd(multipart_parser_t* p);
	static int bodyEnd(multipart_parser_t* p);

private:
	static multipart_parser_settings_t settings;

	String headerName; ///< Current header field name

	HttpRequest* request = nullptr;

	multipart_parser_t* parser = nullptr;
	String name; // current parameter name
};


size_t formMultipartParser(HttpRequest& request, const char* at, int length);
