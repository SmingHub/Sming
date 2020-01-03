/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MultipartParser.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "MultipartParser.h"
#include <Network/Http/HttpBodyParser.h>

multipart_parser_settings_t MultipartParser::settings = {
	.on_header_field = readHeaderName,
	.on_header_value = readHeaderValue,
	.on_part_data = partData,
	.on_part_data_begin = partBegin,
	.on_headers_complete = partHeadersComplete,
	.on_part_data_end = partEnd,
	.on_body_end = bodyEnd,
};


size_t formMultipartParser(HttpRequest& request, const char* at, int length)
{
	auto parser = static_cast<MultipartParser*>(request.args);

	if(length == PARSE_DATASTART) {
		delete parser;

		parser = new MultipartParser(&request);
		request.args = parser;

		return 0;
	}

	if(parser == nullptr) {
		debug_e("Invalid request argument");
		return 0;
	}

	if(length == PARSE_DATAEND) {
		delete parser;
		request.args = nullptr;

		return 0;
	}

	return parser->execute(at, length);
}


/** @brief Boilerplate code for multipart_parser callbacks
 *  @note Obtain parser object and check it
 */
#define GET_PARSER()                                                                                                   \
	auto parser = static_cast<MultipartParser*>(p->data);                                                              \
	if(parser == nullptr) {                                                                                            \
		return -1;                                                                                                     \
	}

MultipartParser::MultipartParser(HttpRequest* request)
{
	if(request->headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		// Content-Type: multipart/form-data; boundary=------------------------a48863c0572edce6
		int startPost = request->headers[HTTP_HEADER_CONTENT_TYPE].indexOf("boundary=");
		if(startPost == -1) {
			return;
		}

		startPost += 9;
		String boundary = "--" + request->headers[HTTP_HEADER_CONTENT_TYPE].substring(startPost);
		parser = multipart_parser_init(boundary.c_str(), &settings);
		parser->data = this;
	}

	this->request = request;
}

MultipartParser::~MultipartParser()
{
	multipart_parser_free(parser);
	parser = nullptr;
}

size_t MultipartParser::execute(const char* at, size_t length)
{
	return multipart_parser_execute(parser, at, length);
}

int MultipartParser::partBegin(multipart_parser_t* p)
{
	GET_PARSER();

	parser->headerName = nullptr;
	parser->headerValue = nullptr;
	parser->stream = nullptr;

	return 0;
}

int MultipartParser::readHeaderName(multipart_parser_t* p, const char* at, size_t length)
{
	GET_PARSER();

	if (parser->headerValue) {
		// process previous header
		int result = parser->processHeader();
		if (result != 0) {
			return result;
		}
	}

	parser->headerName.concat(at, length);
	
	return 0;
}

int MultipartParser::processHeader()
{
	if(headerName == _F("Content-Disposition")) {
		// Content-Disposition: form-data; name="image"; filename=".gitignore"
		// Content-Disposition: form-data; name="data"
		int startPos = headerValue.indexOf(_F("name="));
		if(startPos < 0) {
			debug_e("Invalid header content");
			return -1; // Invalid header content
		}
		startPos += 6; // name="
		int endPos = headerValue.indexOf(';', startPos);
		
		String name;
		if(endPos < 0) {
			name = headerValue.substring(startPos, headerValue.length() - 1);
		} else {
			name = headerValue.substring(startPos, endPos - 1);
		}
		// get stream corresponding to field name
		stream = request->files[name];
	}
	
	headerName = nullptr;
	headerValue = nullptr;

	return 0;
}

int MultipartParser::readHeaderValue(multipart_parser_t* p, const char* at, size_t length)
{
	GET_PARSER();

	parser->headerValue.concat(at, length);

	return 0;
}

int MultipartParser::partHeadersComplete(multipart_parser_t *p) 
{
	GET_PARSER();

	return parser->processHeader();
}

int MultipartParser::partData(multipart_parser_t* p, const char* at, size_t length)
{
	GET_PARSER();

	if(parser->stream != nullptr) {
		size_t written = parser->stream->write((uint8_t*)at, length);
		if(written != length) {
			return 1;
		}
	}

	return 0;
}

int MultipartParser::partEnd(multipart_parser_t* p)
{
	GET_PARSER();

	return 0;
}

int MultipartParser::bodyEnd(multipart_parser_t* p)
{
	GET_PARSER();

	return 0;
}
