/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BasicHttpHeaders.cpp
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "BasicHttpHeaders.h"

#define GET_HEADERS()                                                                                                  \
	auto headers = static_cast<BasicHttpHeaders*>(parser->data);                                                       \
	if(headers == nullptr) {                                                                                           \
		return -1;                                                                                                     \
	}

const http_parser_settings BasicHttpHeaders::parserSettings PROGMEM = {
	.on_message_begin = nullptr,
	.on_url = nullptr,
	.on_status = nullptr,
	.on_header_field = staticOnField,
	.on_header_value = staticOnValue,
	.on_headers_complete = nullptr,
	.on_body = nullptr,
	.on_message_complete = nullptr,
	.on_chunk_header = nullptr,
	.on_chunk_complete = nullptr,
};

void BasicHttpHeaders::clear()
{
	http_parser_init(&parser, HTTP_BOTH);
	parser.data = this;
	count_ = 0;
}

http_errno BasicHttpHeaders::parse(char* data, size_t len, http_parser_type type)
{
	http_parser_init(&parser, type);
	http_parser_execute(&parser, &parserSettings, data, len);
	return http_errno(parser.http_errno);
}

int BasicHttpHeaders::staticOnField(http_parser* parser, const char* at, size_t length)
{
	GET_HEADERS();
	return headers->onField(at, length);
}

int BasicHttpHeaders::onField(const char* at, size_t length)
{
	if(count_ >= ARRAY_SIZE(headers)) {
		return -1;
	}
	const_cast<char*>(at)[length] = '\0';
	headers[count_].name = at;
	return 0;
}

int BasicHttpHeaders::staticOnValue(http_parser* parser, const char* at, size_t length)
{
	GET_HEADERS();
	return headers->onValue(at, length);
}

int BasicHttpHeaders::onValue(const char* at, size_t length)
{
	if(count_ >= ARRAY_SIZE(headers)) {
		return -1;
	}
	const_cast<char*>(at)[length] = '\0';
	headers[count_].value = at;
	++count_;
	return 0;
}

const char* BasicHttpHeaders::operator[](const char* name) const
{
	if(name == nullptr) {
		return nullptr;
	}

	for(unsigned i = 0; i < count_; ++i) {
		auto& hdr = headers[i];
		if(strcasecmp(hdr.name, name) == 0) {
			return hdr.value;
		}
	}

	return nullptr;
}

const char*& BasicHttpHeaders::operator[](const char* name)
{
	if(name == nullptr) {
		nullValue = nullptr;
		return nullValue;
	}

	for(unsigned i = 0; i < count_; ++i) {
		auto& hdr = headers[i];
		if(strcasecmp(hdr.name, name) == 0) {
			return hdr.value;
		}
	}

	if(count_ >= ARRAY_SIZE(headers)) {
		nullValue = nullptr;
		return nullValue;
	}

	auto& hdr = headers[count_];
	hdr.name = name;
	hdr.value = nullptr;
	++count_;
	return hdr.value;
}
