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
#include "Data/Stream/TemplateStream.h"

HttpResponse* HttpResponse::setContentType(const String& type)
{
	headers[HTTP_HEADER_CONTENT_TYPE] = type;
	return this;
}

HttpResponse* HttpResponse::setContentType(enum MimeType type)
{
	return setContentType(ContentType::toString(type));
}

HttpResponse* HttpResponse::setCookie(const String& name, const String& value)
{
	headers[HTTP_HEADER_SET_COOKIE] = name + '=' + value;
	return this;
}

HttpResponse* HttpResponse::setCache(int maxAgeSeconds, bool isPublic)
{
	String cache = isPublic ? F("public") : F("private");
	cache += F(", max-age=") + String(maxAgeSeconds) + F(", must-revalidate");
	headers[HTTP_HEADER_CACHE_CONTROL] = cache;
	return this;
}

HttpResponse* HttpResponse::setAllowCrossDomainOrigin(const String& controlAllowOrigin)
{
	headers[HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN] = controlAllowOrigin;
	return this;
}

HttpResponse* HttpResponse::setHeader(const String& name, const String& value)
{
	headers[name] = value;
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

bool HttpResponse::sendFile(String fileName, bool allowGzipFileCheck)
{
	String compressed = fileName + ".gz";
	if(allowGzipFileCheck && fileExist(compressed)) {
		debug_d("found %s", compressed.c_str());
		setStream(new FileStream(compressed));
		headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
	} else if(fileExist(fileName)) {
		debug_d("found %s", fileName.c_str());
		setStream(new FileStream(fileName));
	} else {
		setStream(nullptr);
		code = HTTP_STATUS_NOT_FOUND;
		return false;
	}

	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		String mime = ContentType::fromFullFileName(fileName);
		if(mime)
			setContentType(mime);
	}

	return true;
}

bool HttpResponse::sendTemplate(TemplateStream* newTemplateInstance)
{
	setStream(newTemplateInstance);
	if(!newTemplateInstance->isValid()) {
		code = HTTP_STATUS_NOT_FOUND;
		freeStreams();
		return false;
	}

	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		String mime = ContentType::fromFullFileName(newTemplateInstance->getName());
		if(mime) {
			setContentType(mime);
		}
	}

	if(!headers.contains(HTTP_HEADER_TRANSFER_ENCODING) && stream->available() < 0) {
		headers[HTTP_HEADER_TRANSFER_ENCODING] = _F("chunked");
	}

	return true;
}

bool HttpResponse::sendDataStream(IDataSourceStream* newDataStream, const String& reqContentType)
{
	setStream(newDataStream);

	if(reqContentType) {
		setContentType(reqContentType);
	}

	return true;
}

String HttpResponse::getBody()
{
	if(stream == nullptr) {
		return nullptr;
	}

	String ret;
	if(stream->available() > 0 && stream->getStreamType() == eSST_Memory) {
		char buf[1024];
		while(stream->available() > 0) {
			int available = stream->readMemoryBlock(buf, 1024);
			stream->seek(available);
			ret += String(buf, available);
			if(available < 1024) {
				break;
			}
		}
	}
	return ret;
}

void HttpResponse::reset()
{
	code = HTTP_STATUS_OK;
	headers.clear();
	freeStreams();
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
