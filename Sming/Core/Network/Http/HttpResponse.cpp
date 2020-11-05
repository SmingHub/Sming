/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpResponse.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpResponse.h"
#include "../WebConstants.h"
#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/FileStream.h"

HttpResponse* HttpResponse::setCookie(const String& name, const String& value)
{
	String s = name;
	s += '=';
	s += value;
	headers[HTTP_HEADER_SET_COOKIE] = s;
	return this;
}

HttpResponse* HttpResponse::setCache(int maxAgeSeconds, bool isPublic)
{
	String cache = isPublic ? F("public") : F("private");
	cache += F(", max-age=");
	cache += maxAgeSeconds;
	cache += F(", must-revalidate");
	headers[HTTP_HEADER_CACHE_CONTROL] = cache;
	return this;
}

bool HttpResponse::sendString(const String& text)
{
	auto memoryStream = new MemoryDataStream();
	if(memoryStream == nullptr) {
		return false;
	}

	setStream(memoryStream);

	return memoryStream->print(text) == text.length();
}

bool HttpResponse::sendString(String&& text) noexcept
{
	auto memoryStream = new MemoryDataStream(std::move(text));
	if(memoryStream == nullptr) {
		return false;
	}
	setStream(memoryStream);
	return true;
}

bool HttpResponse::sendFile(const String& fileName, bool allowGzipFileCheck)
{
	IDataSourceStream* stream = nullptr;
	String compressed = fileName + ".gz";
	if(allowGzipFileCheck && fileExist(compressed)) {
		debug_d("found %s", compressed.c_str());
		stream = new FileStream(compressed);
		headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
	} else if(fileExist(fileName)) {
		debug_d("found %s", fileName.c_str());
		stream = new FileStream(fileName);
	}

	headers[HTTP_HEADER_CONTENT_TYPE] = ContentType::fromFullFileName(fileName);

	return sendNamedStream(stream);
}

bool HttpResponse::sendNamedStream(IDataSourceStream* newDataStream)
{
	String contentType;
	if(newDataStream != nullptr && !headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		contentType = ContentType::fromFullFileName(newDataStream->getName());
	}

	return sendDataStream(newDataStream, contentType);
}

bool HttpResponse::sendDataStream(IDataSourceStream* newDataStream, const String& reqContentType)
{
	setStream(newDataStream);
	if(newDataStream == nullptr || !newDataStream->isValid()) {
		code = HTTP_STATUS_NOT_FOUND;
		freeStreams();
		return false;
	}

	if(reqContentType) {
		setContentType(reqContentType);
	}

	if(!headers.contains(HTTP_HEADER_TRANSFER_ENCODING) && stream->available() < 0) {
		headers[HTTP_HEADER_TRANSFER_ENCODING] = _F("chunked");
	}

	return true;
}

void HttpResponse::reset()
{
	code = HTTP_STATUS_OK;
	headers.clear();
	freeStreams();
}

String HttpResponse::toString() const
{
	String content;
	content += F("HTTP/1.1 ");
	content += unsigned(code);
	content += ' ';
	content += ::toString(code);
	content += " \r\n";
	for(unsigned i = 0; i < headers.count(); i++) {
		content += headers[i];
	}

	if(stream != nullptr && stream->available() >= 0) {
		content += headers.toString(HTTP_HEADER_CONTENT_LENGTH, String(stream->available()));
	} else {
		content += "\r\n";
	}

	return content;
}

void HttpResponse::freeStreams()
{
	// Consistency check
	if(buffer != nullptr) {
		if(buffer != stream) {
			debug_e("HttpResponse: buffer doesn't match stream");
			delete buffer;
		}
		buffer = nullptr;
	}

	delete stream;
	stream = nullptr;
}

void HttpResponse::setBuffer(ReadWriteStream* buffer)
{
	if(buffer == this->buffer) {
		return;
	}

	// Must set stream first
	setStream(buffer);
	// Now safe to set buffer
	this->buffer = buffer;
}

void HttpResponse::setStream(IDataSourceStream* stream)
{
	if(stream == this->stream) {
		return;
	}

	if(this->stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		freeStreams();
	}
	this->stream = stream;
}
