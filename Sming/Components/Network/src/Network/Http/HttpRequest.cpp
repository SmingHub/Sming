/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpRequest.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpRequest.h"
#include "Data/Stream/MemoryDataStream.h"

HttpRequest* HttpRequest::setResponseStream(ReadWriteStream* stream)
{
	if(responseStream != nullptr) {
		debug_e("HttpRequest::setResponseStream: Discarding already set stream!");
		delete responseStream;
	}

	responseStream = stream;
	return this;
}

HttpRequest* HttpRequest::setBody(const uint8_t* rawData, size_t length)
{
	auto memory = new MemoryDataStream();
	auto written = memory->write(rawData, length);
	if(written < length) {
		debug_e("HttpRequest::setBody: Unable to store the complete body");
	}

	return setBody(memory);
}

HttpRequest* HttpRequest::setBody(String&& body) noexcept
{
	return setBody(new MemoryDataStream(std::move(body)));
}

HttpRequest* HttpRequest::setBody(IDataSourceStream* stream)
{
	if(bodyStream != nullptr) {
		debug_e("HttpRequest::setBody: Discarding already set stream!");
		delete bodyStream;
	}

	bodyStream = stream;
	return this;
}

void HttpRequest::reset()
{
	delete queryParams;
	queryParams = nullptr;

	delete bodyStream;
	bodyStream = nullptr;

	delete responseStream;
	responseStream = nullptr;

	postParams.clear();
	files.clear();
	headers.clear();
}

String HttpRequest::toString() const
{
	String content;
	content += ::toString(method);
	content += ' ';
	content += uri.getPathWithQuery();
	content += _F(" HTTP/1.1\r\n");
	if(!headers.contains(HTTP_HEADER_HOST)) {
		content += headers.toString(HTTP_HEADER_HOST, uri.getHostWithPort());
	}
	for(auto hdr : headers) {
		content += hdr;
	}

	if(!headers.contains(HTTP_HEADER_CONTENT_LENGTH)) {
		if(bodyStream == nullptr) {
			content += headers.toString(HTTP_HEADER_CONTENT_LENGTH, "0");
		} else if(bodyStream->available() >= 0) {
			content += headers.toString(HTTP_HEADER_CONTENT_LENGTH, String(bodyStream->available()));
		}
	}
	content += "\r\n";

	return content;
}
