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

		parser = MultipartParser::create(request);
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


MultipartParser::MultipartParser(HttpRequest& request, const String& boundary) 
	: request(request), boundary(boundary)
{
	// Note: Storing the request by reference makes this object noncopyable and as a result the underlying memory of the boundary string does not change throughout the lifetime of this object.
	multipart_parser_init(&parserEngine, boundary.c_str(), boundary.length(), &settings);
	parserEngine.data = this;
}

MultipartParser *MultipartParser::create(HttpRequest& request)
{
	if(request.headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		// Content-Type: multipart/form-data; boundary=------------------------a48863c0572edce6
		int startPost = request.headers[HTTP_HEADER_CONTENT_TYPE].indexOf(FS("boundary="));
		if(startPost >= 0) {
			startPost += 9;

			return new MultipartParser(request, "--" + request.headers[HTTP_HEADER_CONTENT_TYPE].substring(startPost));
		}
	}
	return nullptr;
}

size_t MultipartParser::execute(const char* at, size_t length)
{
	return multipart_parser_execute(&parserEngine, at, length);
}

int MultipartParser::partBegin(multipart_parser_t* p)
{
	GET_PARSER();

	parser->headerName.setLength(0);
	parser->headerValue.setLength(0);
	parser->stream = nullptr;

	return 0;
}

int MultipartParser::readHeaderName(multipart_parser_t* p, const char* at, size_t length)
{
	GET_PARSER();

	if (parser->headerValue != String::empty) {
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
	if(FS("Content-Disposition") == headerName) {
		// Content-Disposition: form-data; name="image"; filename=".gitignore"
		// Content-Disposition: form-data; name="data"
		int startPos = headerValue.indexOf(FS("name="));
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
		stream = request.files[name];
	}

	headerName.setLength(0);
	headerValue.setLength(0);

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
