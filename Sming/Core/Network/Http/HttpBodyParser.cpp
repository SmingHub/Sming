/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpBodyParser.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 * 	Original author
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 * 	Implemented un-escaping of incoming parameters
 *
 ****/

#include "HttpBodyParser.h"
#include "Network/WebHelpers/escape.h"

/*
 * Content is received in chunks which we need to reassemble into name=value pairs.
 * This structure stores the temporary values during parsing.
 */
typedef struct {
	char searchChar = '=';
	String postName;
	String postValue;
} FormUrlParserState;

/*
 * The incoming URL is parsed
 */
void formUrlParser(HttpRequest& request, const char* at, int length)
{
	auto state = static_cast<FormUrlParserState*>(request.args);

	if(length == PARSE_DATASTART) {
		delete state;
		request.args = new FormUrlParserState;
		return;
	}

	assert(state != nullptr);

	auto& params = request.postParams;

	if(length == PARSE_DATAEND) {
		// Store last parameter, if there is one
		if(state->postName.length() != 0) {
			uri_unescape_inplace(state->postValue);
			params[state->postName] = state->postValue;
		}

		delete state;
		request.args = nullptr;

		return;
	}

	if(state == nullptr) {
		debug_e("Invalid request argument");
		return;
	}

	while(length > 0) {
		// Look for search character ('=' or '&') in received text
		auto found = static_cast<const char*>(memchr(at, state->searchChar, length));
		unsigned foundLength = (found == nullptr) ? length : (found - at);

		if(foundLength != 0) {
			if(state->searchChar == '=') {
				state->postName.concat(at, foundLength);
			} else {
				state->postValue.concat(at, foundLength);
			}
		}

		if(found == nullptr) {
			break;
		}

		if(state->searchChar == '=') {
			uri_unescape_inplace(state->postName);
			state->searchChar = '&';
		} else {
			uri_unescape_inplace(state->postValue);
			params[state->postName] = state->postValue;
			state->searchChar = '=';
			// Keep String memory allocated, but clear content
			state->postName.setLength(0);
			state->postValue.setLength(0);
		}
		++foundLength; // Skip the '=' or '&'
		at += foundLength;
		length -= foundLength;
	}
}

void bodyToStringParser(HttpRequest& request, const char* at, int length)
{
	auto data = static_cast<String*>(request.args);

	if(length == PARSE_DATASTART) {
		delete data;
		data = new String();
		request.args = data;
		return;
	}

	if(data == nullptr) {
		debug_e("Invalid request argument");
		return;
	}

	if(length == PARSE_DATAEND || length < 0) {
		request.setBody(*data);
		delete data;
		request.args = nullptr;
		return;
	}

	data->concat(at, length);
}

#ifdef ENABLE_HTTP_SERVER_MULTIPART
#include "multipart-parser/multipart_parser.h"

/** @brief Boilerplate code for multipart_parser callbacks
 *  @note Obtain parser object and check it
 */
#define GET_PARSER()                                                                                                   \
	auto parser = static_cast<MultipartParser*>(p->data);                                                              \
	if(parser == nullptr) {                                                                                            \
		return -1;                                                                                                     \
	}

class MultipartParser
{
public:
	MultipartParser(HttpRequest* request);
	~MultipartParser();

	void execute(const char* at, size_t length);

	static int readHeaderName(multipart_parser_t* p, const char* at, size_t length);
	static int readHeaderValue(multipart_parser_t* p, const char* at, size_t length);
	static int partBegin(multipart_parser_t* p);
	static int partData(multipart_parser_t* p, const char* at, size_t length);
	static int partEnd(multipart_parser_t* p);
	static int bodyEnd(multipart_parser_t* p);

private:
	multipart_parser_settings_t settings;

	bool useValue = false;

	HttpRequest* request;

	multipart_parser_t* parser;
	String name; // current parameter name
};

MultipartParser::MultipartParser(HttpRequest* request)
{
	memset(&settings, 0, sizeof(settings));
	settings.on_header_field = readHeaderName;
	settings.on_header_value = readHeaderValue;
	settings.on_part_data_begin = partBegin;
	settings.on_part_data = partData;
	settings.on_part_data_end = partEnd;
	settings.on_body_end = bodyEnd;

	if(request->headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		// Content-Type: multipart/form-data; boundary=------------------------a48863c0572edce6
		int startPost = request->headers[HTTP_HEADER_CONTENT_TYPE].indexOf("boundary=");
		if(startPost == -1) {
			return;
		}

		startPost += 9;
		String boundary = "--" + request->headers[HTTP_HEADER_CONTENT_TYPE].substring(startPost);
		parser = multipart_parser_init(boundary.c_str(), &settings);
	}

	this->request = request;
	parser->data = this;
}

MultipartParser::~MultipartParser()
{
	multipart_parser_free(parser);
	parser = nullptr;
}

void MultipartParser::execute(const char* at, size_t length)
{
	multipart_parser_execute(parser, at, length);
}

int MultipartParser::partBegin(multipart_parser_t* p)
{
	GET_PARSER();

	parser->useValue = false;
	return 0;
}

int MultipartParser::readHeaderName(multipart_parser_t* p, const char* at, size_t length)
{
	GET_PARSER();

	if(memcmp(at, "Content-Disposition", length) == 0) {
		parser->useValue = true;
	}

	return 0;
}

int MultipartParser::readHeaderValue(multipart_parser_t* p, const char* at, size_t length)
{
	GET_PARSER();

	if(parser->useValue) {
		// Content-Disposition: form-data; name="image"; filename=".gitignore"
		// Content-Disposition: form-data; name="data"
		String value = String(at, length);
		int startPos = value.indexOf("name=");
		if(startPos == -1) {
			return -1; // Invalid header content
		}
		startPos += 6; // name="
		int endPos = value.indexOf(';', startPos);
		if(endPos == -1) {
			parser->name = value.substring(startPos, value.length() - 1);
		} else {
			parser->name = value.substring(startPos, endPos - 1);
		}
	}

	return 0;
}

int MultipartParser::partData(multipart_parser_t* p, const char* at, size_t length)
{
	GET_PARSER();

	ReadWriteStream* stream = parser->request->files[parser->name];
	if(stream != nullptr) {
		stream->write((uint8_t*)at, length);
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

void formMultipartParser(HttpRequest& request, const char* at, int length)
{
	MultipartParser* parser = (MultipartParser*)request.args;

	if(length == -1) {
		delete parser;

		parser = new MultipartParser(&request);
		request.args = parser;

		return;
	}

	if(length == -2) {
		delete parser;
		request.args = nullptr;

		return;
	}

	parser->execute(at, length);
}
#endif /* ENABLE_HTTP_SERVER_MULTIPART */
